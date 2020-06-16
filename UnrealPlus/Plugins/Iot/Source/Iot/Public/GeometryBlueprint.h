// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GeometryBlueprint.generated.h"

//"BlueprintType" is essential to include
UENUM(BlueprintType)
enum class EVisualStyleEnum : uint8
{
	VE_StyleLine 		UMETA(DisplayName = "VisualLine"),
	VE_StylePillar 		UMETA(DisplayName = "VisualPillar"),
};

UENUM(BlueprintType)
enum class EVisualAreaEnum : uint8
{
	VE_AreaNone			UMETA(DisplayName = "VisualNone"),
	VE_AreaCircular		UMETA(DisplayName = "VisualCircular"),
	VE_AreaSquare		UMETA(DisplayName = "VisualSquare"),
};

/**
 * 
 */
UCLASS()
class IOT_API UGeometryBlueprint : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	//LineTraceFromScreen
	UFUNCTION(BlueprintCallable, Category = "Geometry")
		static bool LineTraceFromScreen(APlayerController* pc, const TArray<AActor*>& ActorsIgnores, FHitResult& OutHit);

	//BreakHitResultToFragment
	UFUNCTION(BlueprintCallable, Category = "Geometry")
		static void BreakHitResultToFragment(const FHitResult& OutHit, TArray<FVector>& Verts, int32& Vertex, int32& EdgeStart, int32& EdgeEnd);

	//Draw debug geometry
	UFUNCTION(BlueprintCallable, Category = "Rendering|Debug", meta = (WorldContext = "WorldContextObject", DevelopmentOnly))
		static void DrawDebugMesh(UObject* WorldContextObject, TArray<FVector> const& Verts, TArray<int32> const& Indices, FLinearColor Color = FLinearColor::White, float Duration = 0.f);

	/*
	Draw debug height map
	Parameters:
	Data:			Data.Z contains the height and will be format by Format
	Elevation:		the minimum height
	ElevationValue:	the minimum value at elevation plane
	TopValue:		value at top plane
	FillDensity:	generate sparse values by the density
	Atenuation:		for sparse values lerp
	Format:			use to format value and display
	Thickness:		draw line or pillar when thickness >= 0
	*/
	UFUNCTION(BlueprintCallable, Category = "Rendering|Debug", meta = (WorldContext = "WorldContextObject", DevelopmentOnly))
		static void DrawDebugHeightMap(UObject* WorldContextObject, TArray<FVector> const& Data, float HighPlane, float LowPlane = 0.f, float ElevationPlane = 0.f, float HightValue = 1.f, float LowValue = 0.f
			, EVisualStyleEnum VisualStyle = EVisualStyleEnum::VE_StyleLine, FLinearColor ColorHigh = FLinearColor::Red, FLinearColor ColorLow = FLinearColor::Blue
			, float FillDensity = 0.f, FLinearColor AnnotationColor = FLinearColor::Gray, float AnnotationOffset = 100.f, float Attenuation = 0.5f, FString Format = "%0.2f", float PointSize = 0.1f, float PillarSize = 0.1f
			, bool bPersistent = false, float Duration = 1.f, EVisualAreaEnum VisualArea = EVisualAreaEnum::VE_AreaNone);
};
