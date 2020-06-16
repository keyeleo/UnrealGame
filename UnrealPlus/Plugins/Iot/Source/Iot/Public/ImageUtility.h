// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ImageUtility.generated.h"

/**
 * 
 */
UCLASS()
class IOT_API UImageUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, Category = "Image")
		static void ScreenShot(const FString& FileName = TEXT("screenshot"), bool WithUI = true, bool Suffix = true);
};
