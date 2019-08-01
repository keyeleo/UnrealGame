// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBlueprintFunctionLibrary.h"

#include "Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "Misc/Paths.h"
#include "IPlatformFilePak.h"

bool UMyBlueprintFunctionLibrary::LoadPakFile(TArray<FString>& AssetsList, FString FileName, FString MountPoint) {
	// Use FPaths::ProjectContentDir() as root
	MountPoint = FPaths::ProjectContentDir() + MountPoint;
	if (!MountPoint.EndsWith(TEXT("/")))
		MountPoint += TEXT("/");

	// mount as same as the input directory, OR not as well
	FileName = MountPoint + FileName;
	UE_LOG(LogTemp, Warning, TEXT("MountPoint=%s, FileName=%s"), *MountPoint, *FileName);

	//获取当前使用的平台,这里使用的是WIN64平台
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	//初始化PakPlatformFile
	FPakPlatformFile* PakPlatformFile = new FPakPlatformFile();
	PakPlatformFile->Initialize(&PlatformFile, TEXT(""));
	FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);

	//获取Pak文件
	FPakFile PakFile(&PlatformFile, *FileName, false);

	//设置pak文件的Mount点. 
	PakFile.SetMountPoint(*MountPoint);

	//对pak文件mount到前面设定的MountPoint
	UObject* LoadObject = nullptr;
	if (PakPlatformFile->Mount(*FileName, 0, *MountPoint)) {
		//得到Pak文件中MountPoint路径下的所有文件
		PakFile.FindFilesAtPath(AssetsList, *PakFile.GetMountPoint(), true, false, true);
		UE_LOG(LogTemp, Warning, TEXT("Mount %d assets in pak %s"), AssetsList.Num(), *FileName);
		for (auto& AssetName : AssetsList) {
			//remove extra path
			AssetName = AssetName.Replace(*FPaths::ProjectContentDir(), TEXT(""));
		}
		return true;
	}
	return false;
}

UObject* UMyBlueprintFunctionLibrary::LoadObject(FString AssetName, FString Name) {
	//对文件的路径进行处理,转换成StaticLoadObject的那种路径格式
	UE_LOG(LogTemp, Warning, TEXT("Load %s"), *AssetName);
	FString LeftStr, RightStr;
	AssetName.Split(TEXT("."), &LeftStr, &RightStr);		// [ path/xxx.uasset ]
	if (RightStr.Equals(TEXT("uasset"))) {
		AssetName = TEXT("/Game/") + LeftStr;				// package name [ xxx ]
		AssetName += TEXT(".") + (Name.IsEmpty() ? FPackageName::GetShortName(LeftStr) : Name);	// asset name [ xxx.xxx ]
		FStringAssetReference reference = AssetName;
		UE_LOG(LogTemp, Warning, TEXT("Load object %s"), *AssetName);

		//加载UObject
		FStreamableManager StreamableManager;
		return StreamableManager.LoadSynchronous(reference);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Ignore asset %s"), *AssetName);
	return nullptr;
}
