// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyUtils.generated.h"

/**
 * 
 */
UCLASS()
class UMyUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable,Category = "Utils")
    static FHitResult RayGetHitResult(AActor* actor, FVector TraceStart, FVector TraceEnd);

    UFUNCTION(BlueprintCallable,Category = "Utils")
    static bool LoadStringFromFile(FString& str, const FString& file);
    
    UFUNCTION(BlueprintCallable,Category = "Utils")
    static void SaveStringToFile(const FString& str, const FString& file);
};
