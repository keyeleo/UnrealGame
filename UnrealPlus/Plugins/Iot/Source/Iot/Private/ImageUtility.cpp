// Fill out your copyright notice in the Description page of Project Settings.


#include "ImageUtility.h"

#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "ModuleManager.h"
#include "FileHelper.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"

void UImageUtility::ScreenShot(const FString& FileName, bool WithUI, bool Suffix) {
	auto now = FDateTime::Now();
	FString filename = Suffix ? FString::Printf(*(FileName + TEXT("_") + TEXT("%d%02d%02d%02d%02d%03d")), now.GetYear(), now.GetMonth(), now.GetDay(), now.GetHour(), now.GetMinute(), now.GetMillisecond()) : FileName;
	if (!UGameViewportClient::OnScreenshotCaptured().IsBound())
	{
		if (WithUI)
			FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() + "ScreenShots/Windows/" + filename, WithUI, false);
		else {
			FVector2D Size;
			GEngine->GameViewport->GetViewportSize(Size);
			FString Param = FString::Printf(TEXT("HighResShot %dx%d filename=%s"), (int)Size.X, (int)Size.Y, *filename);
			GEngine->GameViewport->Exec(nullptr, *Param, *GLog);
		}
	}
}
