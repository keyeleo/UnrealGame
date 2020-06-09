// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MediaActor.generated.h"

class UMediaPlayer;
class UMediaTexture;

UCLASS()
class FFMPEGMEDIA_API AMediaActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMediaActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void CreateBind();
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Media|MediaPlayer")
		void Bind(UMaterialInterface* MaterialInterface);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MediaTexture")
		UMediaPlayer* MediaPlayer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MediaTexture")
		UMediaTexture* MediaTexture;
};
