// Fill out your copyright notice in the Description page of Project Settings.


#include "WidgetPainting.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Rendering/DrawElements.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateBrushAsset.h"


void UWidgetPainting::DrawConvex(FPaintContext& Context, USlateBrushAsset* Brush, const TArray<FVector2D>& Points, FLinearColor Tint) {
	TArray<int32> InIndexes;
	for (int i = 1, ii = Points.Num() - 1; i < ii; ++i) {
		InIndexes.Add(0);
		InIndexes.Add(i);
		InIndexes.Add(i+1);
	}

	DrawPolygon(Context, Brush, Points, InIndexes, Tint);
}

void UWidgetPainting::DrawPolygon(FPaintContext& Context, USlateBrushAsset* Brush, const TArray<FVector2D>& InVerts, const TArray<int32>& InIndexes, FLinearColor Tint) {
	/*
	if (Brush == nullptr || InVerts.Num() < 3 || InIndexes.Num() < 3)
		return;

	//const FSlateBrush* MyBrush = FCoreStyle::Get().GetBrush("ColorWheel.HueValueCircle");
	const FSlateBrush* MyBrush = &Brush->Brush;	
	// @todo this is not the correct way to do this
	FSlateShaderResourceProxy* ResourceProxy = FSlateDataPayload::ResourceManager->GetShaderResource(*MyBrush);
	FSlateResourceHandle Handle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*MyBrush);

	//make UVs
	FVector2D Min(INFINITY, INFINITY), Max(-INFINITY, -INFINITY), Size(0, 0), StartUV(0, 0), SizeUV(1, 1);
	for (auto& Vertex : InVerts) {
		if (Min.X > Vertex.X) Min.X = Vertex.X;
		if (Min.Y > Vertex.Y) Min.Y = Vertex.Y;
		if (Max.X < Vertex.X) Max.X = Vertex.X;
		if (Max.Y < Vertex.Y) Max.Y = Vertex.Y;
	}
	Size = Max - Min; if (Size.X == 0.f)Size.X = 1.f; if (Size.Y == 0.f)Size.Y = 1.f;
	if (ResourceProxy != nullptr){
		StartUV = ResourceProxy->StartUV;
		SizeUV = ResourceProxy->SizeUV;
	}
	//UE_LOG(LogTemp, Warning, TEXT("StartUV=%s, SizeUV=%s, Min=%s, Size=%s"), *StartUV.ToString(), *SizeUV.ToString(), *Min.ToString(), *Size.ToString());

	// Make a triangle fan in the area allotted
	TArray<SlateIndex> Indexes;
	TArray<FSlateVertex> Verts;
	Verts.Reserve(InVerts.Num());

	auto Offset = Context.AllottedGeometry.GetAbsolutePosition();
	//TODO: apply rotation,	maybe FSlateRenderTransform::AccumulatedRenderTransform;
	for (auto& Vertex : InVerts) {
		Verts.AddZeroed();
		{
			FSlateVertex& NewVert = Verts.Last();
			NewVert.Position = Vertex + Offset;
			NewVert.Color = Tint.ToFColor(false);

			NewVert.TexCoords[0] = StartUV.X + SizeUV.X * (Vertex.X - Min.X) / Size.X;
			NewVert.TexCoords[1] = StartUV.Y + SizeUV.Y * (Vertex.Y - Min.Y) / Size.Y;
			NewVert.TexCoords[2] = NewVert.TexCoords[3] = 1.0f;
			//UE_LOG(LogTemp, Warning, TEXT("Vertex=%s, UVs=%f, %f"), *NewVert.Position.ToString(), NewVert.TexCoords[0], NewVert.TexCoords[1]);
		}
	}
	for (auto Index : InIndexes) {
		Indexes.Add((SlateIndex)Index);
		//UE_LOG(LogTemp, Warning, TEXT("Index=%d"), Index);
	}

	Context.MaxLayer++;
	FSlateDrawElement::MakeCustomVerts(Context.OutDrawElements, Context.MaxLayer, Handle, Verts, Indexes, nullptr, 0, 0);
	*/
}
