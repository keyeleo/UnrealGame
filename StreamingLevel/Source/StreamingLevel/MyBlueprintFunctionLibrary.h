// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class STREAMINGLEVEL_API UMyBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, Category = "AssetManager")
		static void LoadPakFile(TArray<FString>& AssetsList, FString file, FString MountPoint = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "AssetManager")
		static UObject* SyncLoadAsset(FString AssetName, FString Name = TEXT(""));
};
