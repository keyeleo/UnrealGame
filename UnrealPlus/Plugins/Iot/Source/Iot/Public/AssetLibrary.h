// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AssetLibrary.generated.h"

/**
 * 
 */
UCLASS()
class IOT_API UAssetLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
    UFUNCTION(BlueprintCallable, Category = "AssetManager")
    static void LoadPakFile(TArray<FString>& AssetsList, FString file, FString MountPoint = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "AssetManager")
		static UObject* SyncLoadAsset(FString AssetName, FString Name = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "AssetManager")
		static AActor* SpawnActor(AActor* parent, UClass* Class=nullptr, FString ClassName= TEXT(""));
};
