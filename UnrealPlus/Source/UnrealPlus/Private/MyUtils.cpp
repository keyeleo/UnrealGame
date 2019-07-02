// Fill out your copyright notice in the Description page of Project Settings.


#include "MyUtils.h"

#include "Engine.h"
#include "Engine/GameEngine.h"

FHitResult UMyUtils::RayGetHitResult(AActor* actor,FVector TraceStart, FVector TraceEnd){    
    FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(DestopPresenceCursorTrace), true);
    //Hit是用来存储射线的一些数据，如Actor、坐标等等
    FHitResult Hit(ForceInit);
    if (actor && actor->GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams)){
        //绘制射线
        //DrawRayLine(TraceStart, Hit.Location,0.03f);
    }
    return Hit;
}

bool UMyUtils::LoadStringFromFile(FString& str, const FString& file){
    //    UE_LOG(LogInit, Warning, TEXT("LaunchDir=%s, EngineDir=%s, EnterpriseDir=%s, RootDir=%s, ProjectDir=%s, ProjectUserDir=%s")
    //           , *FPaths::LaunchDir(), *FPaths::EngineDir(), *FPaths::EnterpriseDir(), *FPaths::RootDir(), *FPaths::ProjectDir()
    //           , *FPaths::ProjectUserDir());
    
    if (!FPaths::FileExists(*file)){
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("** Could not Find File **"));
        return false;
    }
    auto result=FFileHelper::LoadFileToString(str, *(file) );
    UE_LOG(LogInit, Warning, TEXT("file content=%s"), *str);
    return result;
}

void UMyUtils::SaveStringToFile(const FString& str, const FString& file){
    FFileHelper::SaveStringToFile(str,*file);
}
