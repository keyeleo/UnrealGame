// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Blueprint/UserWidget.h"
#include "WidgetPainting.generated.h"

/**
 * 
 */
UCLASS()
class IOT_API UWidgetPainting : public UBlueprintFunctionLibrary{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Painting")
		static void DrawConvex(UPARAM(ref) FPaintContext& Context, USlateBrushAsset* Brush, const TArray<FVector2D>& Points, FLinearColor Tint = FLinearColor::White);

	UFUNCTION(BlueprintCallable, Category = "Painting")
		static void DrawPolygon(UPARAM(ref) FPaintContext& Context, USlateBrushAsset* Brush, const TArray<FVector2D>& InVerts,const TArray<int32>& InIndexes,FLinearColor Tint= FLinearColor::White);
};

