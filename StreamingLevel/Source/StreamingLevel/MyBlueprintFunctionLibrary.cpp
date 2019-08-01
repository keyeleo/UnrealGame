// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBlueprintFunctionLibrary.h"

#include "Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "Misc/Paths.h"
#include "IPlatformFilePak.h"

UObject* UMyBlueprintFunctionLibrary::LoadActorFromPak(FString FileName, FString MountPoint) {
	// Use FPaths::GameContentDir() as mount point
	if(MountPoint.IsEmpty())
		MountPoint = FPaths::ProjectDir();	//GameContentDir()

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
	UE_LOG(LogTemp, Warning, TEXT("get PakFile..."));
	//设置pak文件的Mount点. 
	PakFile.SetMountPoint(*MountPoint);

	//对pak文件mount到前面设定的MountPoint
	UObject* LoadObject = nullptr;
	if (PakPlatformFile->Mount(*FileName, 0, *MountPoint))
	{
		TArray<FString> FileList;

		//得到Pak文件中MountPoint路径下的所有文件
		PakFile.FindFilesAtPath(FileList, *PakFile.GetMountPoint(), true, false, true);
		UE_LOG(LogTemp, Warning, TEXT("Mount Success, find %d assets"), FileList.Num());
		if (FileList.Num()>0) {
			//对文件的路径进行处理,转换成StaticLoadObject的那种路径格式
			FString LeftStr, RightStr;
			FString AssetName = FileList[0];					// full path
			AssetName = FPackageName::GetShortName(AssetName);	// [ xxx.uasset ]
			AssetName.Split(TEXT("."), &LeftStr, &RightStr);
			AssetName = TEXT("/Game/") + LeftStr;				// package name [ xxx ]
			AssetName+= TEXT(".") + LeftStr;					// asset name [ xxx.xxx ]
			FStringAssetReference reference = AssetName;

			//加载UObject
			FStreamableManager StreamableManager;
			LoadObject = StreamableManager.LoadSynchronous(reference);
			if (LoadObject != nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("Object Load Success... class=%s"),*LoadObject->GetClass()->GetName());

				UStaticMesh* Obj = Cast<UStaticMesh>(LoadObject);
				if(Obj != nullptr)
				{
					//AStaticMeshActor* actor = GetWorld()->SpawnActor<AStaticMeshActor>(FVector::ZeroVector, FRotator::ZeroRotator);
					//actor->GetStaticMeshComponent()->Mobility = EComponentMobility::Movable;
					//actor->GetStaticMeshComponent()->SetStaticMesh(Obj);
				}else
				{
					UE_LOG(LogTemp, Warning, TEXT("Object cast failed..."));
				}
			}else
			{
				UE_LOG(LogTemp, Error, TEXT("Can not Load asset..."));
			}
		}
	}else{
		UE_LOG(LogTemp, Error, TEXT("Mount Failed"));
	}
	return LoadObject;
}
