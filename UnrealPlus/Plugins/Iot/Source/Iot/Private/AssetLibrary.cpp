// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetLibrary.h"


#include "Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "Misc/Paths.h"
#include "IPlatformFilePak.h"

FString UAssetLibrary::GetGameConfig(const FString& Variable, const FString& Section, bool reload) {
	if (!GConfig) return "";

	if(reload)
		FConfigCacheIni::LoadGlobalIniFile(GGameIni, TEXT("Game"), nullptr, true);
	FString Value;
	GConfig->GetString(	*Section,*Variable,	Value,GGameIni);
	return Value;
}

void UAssetLibrary::LoadPakFile(TArray<FString>& AssetsList, FString FileName, FString MountPoint) {
    // Use FPaths::ProjectContentDir() as root
    MountPoint = FPaths::ProjectContentDir() + MountPoint;
    if (!MountPoint.EndsWith(TEXT("/")))
        MountPoint += TEXT("/");
    
    // mount as same as the input directory, OR not as well
    FileName = MountPoint + FileName;
    //UE_LOG(LogTemp, Warning, TEXT("MountPoint=%s, FileName=%s"), *MountPoint, *FileName);
    
    // request platform file and initialize pak file
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    FPakPlatformFile* PakPlatformFile = new FPakPlatformFile();    // leak? but will crash without pointer
    PakPlatformFile->Initialize(&PlatformFile, TEXT(""));
    FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);
    
    // set MountPoint for pak file
    FPakFile PakFile(&PlatformFile, *FileName, false);
    PakFile.SetMountPoint(*MountPoint);
    
    // mount pak file and find all assets
    UObject* LoadObject = nullptr;
    if (PakPlatformFile->Mount(*FileName, 0, *MountPoint)) {
        PakFile.FindFilesAtPath(AssetsList, *PakFile.GetMountPoint(), true, false, true);
        for (auto& AssetName : AssetsList) {
            //remove extra path
            AssetName = AssetName.Replace(*FPaths::ProjectContentDir(), TEXT(""));
        }
        // OR could not save
        if (GIsEditor)PakPlatformFile->Unmount(*FileName);
    }
    else
        UE_LOG(LogTemp, Error, TEXT("Mount %s failed at %s"), *FileName, *MountPoint);
}

UObject* UAssetLibrary::SyncLoadAsset(FString AssetName, FString Name) {
    // translate path as format as for StaticLoadObject
    //UE_LOG(LogTemp, Warning, TEXT("Load %s"), *AssetName);
    FString LeftStr, RightStr;
    AssetName.Split(TEXT("."), &LeftStr, &RightStr);        // [ path/xxx.uasset ]
    if (RightStr.Equals(TEXT("uasset"))) {
        AssetName = TEXT("/Game/") + LeftStr;                // package name [ xxx ]
        AssetName += TEXT(".") + (Name.IsEmpty() ? FPackageName::GetShortName(LeftStr) : Name);    // asset name [ xxx.xxx ]
        FStringAssetReference reference = AssetName;
        //UE_LOG(LogTemp, Warning, TEXT("Load object %s"), *AssetName);
        
        // load UObject
        FStreamableManager StreamableManager;
        return StreamableManager.LoadSynchronous(reference);
    }
    else
        UE_LOG(LogTemp, Warning, TEXT("Ignore asset %s"), *AssetName);
    return nullptr;
}

AActor* UAssetLibrary::SpawnActor(AActor* Parent, UClass* NewClass, FString ClassName) {
	if (!NewClass)
		NewClass = LoadClass<AActor>(nullptr, *ClassName);

	if (NewClass)
	{
		if (NewClass->IsChildOf(AActor::StaticClass()))
		{
			auto World = Parent ? Parent->GetWorld() : GWorld;
			auto actor = World->SpawnActor<AActor>(NewClass, FVector(0, 0, 0), FRotator(0, 0, 0));
			if (actor && Parent)
				actor->AttachToActor(Parent, FAttachmentTransformRules::KeepRelativeTransform);
			return actor;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Class is not derived from Actor. %s"), *NewClass->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to find class."));
	}
	return nullptr;
}
