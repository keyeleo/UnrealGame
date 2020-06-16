// Fill out your copyright notice in the Description page of Project Settings.


#include "GeometryBlueprint.h"

#include "Engine.h"
#include "Kismet/KismetMathLibrary.h"

#include "PhysXPublic.h"
#include "PhysXIncludes.h"

bool UGeometryBlueprint::LineTraceFromScreen(APlayerController* pc, const TArray<AActor*>& ActorsIgnores, FHitResult& OutHit) {
	bool bHit = false;
	//FVector2D CursorPos; GEngine->GameViewport->GetMousePosition(CursorPos);

	if (pc) {
		float x = 0.f, y = 0.f;
		FVector Location, Direction;
		pc->GetMousePosition(x, y);
		pc->DeprojectMousePositionToWorld(Location, Direction);
		//UE_LOG(LogTemp, Warning, TEXT("x=%0.2f, y=%0.2f"), x, y);

		const float TraceLength = 1000.f;
		const FVector& Start = Location;
		FVector End = Location + Direction * TraceLength;

		//TraceTypeQuery1 for Visibility; TraceTypeQuery2 for Camera
		UKismetSystemLibrary::LineTraceSingle(pc->GetWorld(), Start, End, ETraceTypeQuery::TraceTypeQuery1, true, ActorsIgnores, EDrawDebugTrace::None, OutHit, true);
		if (OutHit.bBlockingHit) {
			auto strHit = FString::Printf(
				TEXT("Location:%s Normal:%s Actor:%s Component:%s FaceIndex:%d"),
				*OutHit.Location.ToString(),
				*OutHit.Normal.ToString(),
				OutHit.Actor.IsValid() ? *OutHit.Actor->GetName() : TEXT("None"),
				OutHit.Component.IsValid() ? *OutHit.Component->GetName() : TEXT("None"),
				OutHit.FaceIndex);

			//UE_LOG(LogTemp, Warning, TEXT("OutHit=%s"), *strHit);
		}
		bHit = OutHit.bBlockingHit;
	}
	return bHit;
}

template<typename T> void fillHitResultFragment(TArray<FVector>& TriangleVerts, PxTriangleMesh* EachTriMesh, PxU32 FaceIndex, const FTransform& Transform) {
	const void* Triangles = EachTriMesh->getTriangles();
	const PxVec3* Vertices = EachTriMesh->getVertices();

	auto ptr = (T*)Triangles;
	ptr += FaceIndex * 3;
	auto v0 = Vertices[*ptr++];
	auto v1 = Vertices[*ptr++];
	auto v2 = Vertices[*ptr++];
	TriangleVerts.Add(Transform.TransformPosition(FVector(v0.x, v0.y, v0.z)));
	TriangleVerts.Add(Transform.TransformPosition(FVector(v1.x, v1.y, v1.z)));
	TriangleVerts.Add(Transform.TransformPosition(FVector(v2.x, v2.y, v2.z)));
	//UE_LOG(LogTemp, Warning, TEXT("x=%0.2f, y=%0.2f, z=%0.2f   , x=%0.2f, y=%0.2f, z=%0.2f    , x=%0.2f, y=%0.2f, z=%0.2f"), v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z);
}

void UGeometryBlueprint::BreakHitResultToFragment(const FHitResult& OutHit, TArray<FVector>& Verts, int32& Vertex, int32& EdgeStart, int32& EdgeEnd) {
	if (OutHit.bBlockingHit) {
		auto Component = OutHit.GetComponent();
		if (Component->IsA(UStaticMeshComponent::StaticClass())) {
			const auto& Transform = Component->GetOwner()->GetTransform();
			auto StaticMeshComponent = static_cast<UStaticMeshComponent*>(Component);
			for (PxTriangleMesh* EachTriMesh : StaticMeshComponent->GetBodySetup()->TriMeshes) {
				auto VertexCount = EachTriMesh->getNbTriangles();
				auto TriangleCount = EachTriMesh->getNbTriangles();
				const PxVec3* Vertices = EachTriMesh->getVertices();
				const void* Triangles = EachTriMesh->getTriangles();
				const PxU32* TrianglesRemap = EachTriMesh->getTrianglesRemap();

				// find original face index
				PxU32 FaceIndex = ~0;
				for (PxU32 v = 0; v < VertexCount; ++v) {
					if (TrianglesRemap[v] == OutHit.FaceIndex) {
						FaceIndex = v;
						break;
					}
				}
				if (FaceIndex != ~0) {
					//fill vertices
					//UE_LOG(LogTemp, Warning, TEXT("Meshes=%d, TriangleCount=%d, VertexCount=%d, OriginId=%d"), StaticMeshComponent->GetBodySetup()->TriMeshes.Num(), TriangleCount, VertexCount, TrianglesRemap[OutHit.FaceIndex]);
					auto TriFlag = EachTriMesh->getTriangleMeshFlags();
					if (TriFlag.isSet(physx::PxTriangleMeshFlag::Enum::e16_BIT_INDICES))
						fillHitResultFragment<uint16>(Verts, EachTriMesh, FaceIndex, Transform);
					else
						fillHitResultFragment<uint32>(Verts, EachTriMesh, FaceIndex, Transform);

					//find the nearest edge
					float dist = INFINITY;
					int I = 0;
					for (int i = 0; i < 3; ++i) {
						auto d = UKismetMathLibrary::GetPointDistanceToSegment(OutHit.Location, Verts[i], Verts[(i + 1) % 3]);
						if (dist > d) {
							dist = d;
							I = i;
						}
					}
					EdgeStart = I;
					EdgeEnd = (I + 1) % 3;

					//find the nearest vertex
					dist = INFINITY;
					I = 0;
					for (int i = 0; i < 3; ++i) {
						auto d = UKismetMathLibrary::Vector_Distance(OutHit.Location, Verts[i]);
						if (dist > d) {
							dist = d;
							I = i;
						}
					}
					Vertex = I;

					break;
				}
			}
		}
	}
}
// -------------------------------------------------------
// Draw debug geometry
// -------------------------------------------------------
void UGeometryBlueprint::DrawDebugMesh(UObject* WorldContextObject, TArray<FVector> const& Verts, TArray<int32> const& Indices, FLinearColor Color, float LifeTime) {
#if ENABLE_DRAW_DEBUG
	if (UWorld * World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		::DrawDebugMesh(World, Verts, Indices, Color.ToFColor(true), false, LifeTime, SDPG_World);
	}
#endif
}


void fillSparse(TArray<FVector>& SparseData, TArray<FVector> const& Data, FBox& BoundBox, float FillDensity, EVisualAreaEnum VisualArea) {
	FVector Center, Extents;
	BoundBox.GetCenterAndExtents(Center, Extents);
	float radius = FMath::Sqrt(Extents.X * Extents.X + Extents.Y * Extents.Y);
	for (float x = BoundBox.Min.X; x < BoundBox.Max.X; x += FillDensity) {
		for (float y = BoundBox.Min.Y; y < BoundBox.Max.Y; y += FillDensity) {
			if (VisualArea == EVisualAreaEnum::VE_AreaCircular && x * x + y * y > radius * radius)
				continue;
			//lerp by distance and Elevation
			float z = 0.f;
			float total = 0.f;
			for (auto& p : Data) {
				float distance = (x - p.X) * (x - p.X) + (y - p.Y) * (y - p.Y);
				total += distance;
			}
			float tw = 0.f;
			for (auto& p : Data) {
				float distance = (x - p.X) * (x - p.X) + (y - p.Y) * (y - p.Y);
				if (distance < 0.00001f)distance = 0.00001f;
				tw += total / distance;
			}
			for (auto& p : Data) {
				float distance = (x - p.X) * (x - p.X) + (y - p.Y) * (y - p.Y);
				if (distance < 0.00001f)distance = 0.00001f;
				float w = total / distance /tw;
				z += p.Z * w;
			}
			//lerp the nearest again at last
			SparseData.Add(FVector(x, y, z));
		}
	}
}

void drawValue(UWorld* World, const FVector& Point, float PointSize, float PillarSize, float HighPlane, float LowPlane, float ElevationPlane
	, EVisualStyleEnum VisualStyle, FLinearColor ColorHigh, FLinearColor ColorLow, float FillDensity, FLinearColor AnnotationColor, float AnnotationOffset
	, FString Str, bool bPersistent, float Duration) {
	FVector Start(Point.X, Point.Y, ElevationPlane);
	TArray<FVector> Points;
	TArray<FColor> Colors;

	const int Segment = 10;
	float H = 0.001f + (HighPlane - LowPlane) / Segment;
	float z = ElevationPlane;
	while (z < Point.Z) {
		FVector point(Point.X, Point.Y, z);
		auto linearColor = (z<=LowPlane)? ColorLow: ((z>=HighPlane)? ColorHigh: FLinearColor::LerpUsingHSV(ColorLow, ColorHigh, (z - LowPlane) / (HighPlane - LowPlane)));
		Points.Add(point);
		Colors.Add(linearColor.ToFColor(true));
		z += H;
	}

	Points.Add(Point);
	z = Point.Z;
	auto linearColor = (z <= LowPlane) ? ColorLow : ((z >= HighPlane) ? ColorHigh :	FLinearColor::LerpUsingHSV(ColorLow, ColorHigh, (z - LowPlane) / (HighPlane - LowPlane)));
	Colors.Add(linearColor.ToFColor(true));

	uint8 DepthPriority = 0;
	if (PillarSize > 0.f) {
		for (int i = 0, ii = Points.Num() - 1; i < ii; ++i) {
			auto& start = Points[i];
			auto& end = Points[i + 1];
			auto& color = Colors[i];
			if (VisualStyle == EVisualStyleEnum::VE_StyleLine) {
				::DrawDebugLine(World, start, end, color, bPersistent, Duration, DepthPriority, PillarSize);
			}
			else {
				::DrawDebugCylinder(World, start, end, PillarSize, 4, color, bPersistent, Duration);
			}
		}
	}

	if (Str.Len() > 0) {
		auto End = Point + FVector::UpVector * AnnotationOffset;
		::DrawDebugString(World, End + FVector::UpVector * 10.f, Str, nullptr, Colors.Last(), bPersistent ? -1.f : Duration);
		::DrawDebugLine(World, Point, End, AnnotationColor.ToFColor(true), bPersistent, Duration, DepthPriority);
	}
	if (VisualStyle == EVisualStyleEnum::VE_StyleLine && PointSize > 0.f)
		::DrawDebugPoint(World, Point, PointSize, Colors.Last(), bPersistent, Duration);
}

void UGeometryBlueprint::DrawDebugHeightMap(UObject* WorldContextObject, TArray<FVector> const& Data, float HighPlane, float LowPlane, float ElevationPlane, float HightValue, float LowValue
	, EVisualStyleEnum VisualStyle, FLinearColor ColorHigh, FLinearColor ColorLow, float FillDensity, FLinearColor AnnotationColor, float AnnotationOffset, float Attenuation
	, FString Format, float PointSize, float PillarSize, bool bPersistent, float Duration, EVisualAreaEnum VisualArea) {
	if (Data.Num() <= 0)return;
#if ENABLE_DRAW_DEBUG
	if (UWorld * World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull)) {
		FBox BoundBox = FBox(Data);

		//fill sparse
		TArray<FVector> SparseData;
		if (FillDensity > 0.01f) {
			fillSparse(SparseData, Data, BoundBox, FillDensity, VisualArea);
		}

		//draw all values
		for (auto& Point : Data) {
			FString str;
			if (Format.Len() > 0 && HighPlane!=LowPlane) {
				auto value = LowValue + ((HightValue - LowValue) * (Point.Z-LowPlane))/ (HighPlane-LowPlane);
				str = FString::Printf(*Format, value);
			}
			drawValue(World, Point, PointSize, PillarSize, HighPlane, LowPlane, ElevationPlane, VisualStyle, ColorHigh, ColorLow, FillDensity, AnnotationColor, AnnotationOffset, str, bPersistent, Duration);
		}
		for (auto& Point : SparseData) {
			drawValue(World, Point, PointSize, PillarSize, HighPlane, LowPlane, ElevationPlane, VisualStyle, ColorHigh, ColorLow, FillDensity, AnnotationColor, AnnotationOffset, TEXT(""), bPersistent, Duration);
		}
	}
#endif
}
