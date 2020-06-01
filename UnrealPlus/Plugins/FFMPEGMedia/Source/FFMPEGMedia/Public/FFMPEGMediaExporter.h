// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FFMPEGMediaExporter.generated.h"

/**
 * 
 */
UCLASS()
class FFMPEGMEDIA_API UFFMPEGMediaExporter : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
    UFUNCTION(BlueprintCallable, Category = "Media")
    static void Export(const FString& Url, const FString& OutputFile, int Length=300);
};
