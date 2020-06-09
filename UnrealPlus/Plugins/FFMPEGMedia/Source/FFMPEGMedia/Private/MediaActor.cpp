// Fill out your copyright notice in the Description page of Project Settings.


#include "MediaActor.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Components/Image.h"
#include "WidgetComponent.h"
#include "UserWidget.h"
#include "IMediaEventSink.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "IAssetTools.h"
//#include "MediaPlayerFactoryNew.h"
#include "AssetRegistryModule.h"

// Sets default values
AMediaActor::AMediaActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMediaActor::BeginPlay()
{
	Super::BeginPlay();
	CreateBind();
}

void AMediaActor::CreateBind() {
	MediaPlayer = NewObject<UMediaPlayer>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UMediaPlayer::StaticClass()));
	MediaPlayer->PlayOnOpen = false;
	MediaPlayer->AddToRoot();

	if (MediaPlayer != nullptr){
		FString AssetName = MediaPlayer->GetName() + TEXT("_Video");
		EObjectFlags Flags = RF_Public | RF_Standalone | RF_Transactional;
		UObject* NewObj = NewObject<UObject>(GetTransientPackage(), UMediaTexture::StaticClass(), FName(*AssetName), Flags);
		// Notify the asset registry
		FAssetRegistryModule::AssetCreated(NewObj);
		MediaTexture = Cast<UMediaTexture>(NewObj);

		if (MediaTexture != nullptr)
		{
			MediaTexture->SetDefaultMediaPlayer(MediaPlayer);

			TInlineComponentArray<UActorComponent*> Components;
			GetComponents(Components);
			for (UActorComponent* Component : Components) {
				if (Component->IsA(UStaticMeshComponent::StaticClass())) {
					UStaticMeshComponent* StaticMeshComponent = StaticCast<UStaticMeshComponent*>(Component);
					UMaterialInterface* MaterialInterface = StaticMeshComponent->GetMaterial(0);
					UMaterialInstanceDynamic* mid = UKismetMaterialLibrary::CreateDynamicMaterialInstance(StaticMeshComponent, MaterialInterface);
					StaticMeshComponent->SetMaterial(0, mid);
					mid->SetTextureParameterValue(TEXT("Texture"), MediaTexture);

					//Bind(StaticMeshComponent->GetMaterial(0));
					continue;
				}

				if (Component->IsA(UWidgetComponent::StaticClass())) {
					UWidgetComponent* WidgetComponent = StaticCast<UWidgetComponent*>(Component);
					UUserWidget* UserWidget=WidgetComponent->GetUserWidgetObject();
					TArray<USceneComponent*> Components = WidgetComponent->GetAttachChildren();
					WidgetComponent->GetChildrenComponents(true, Components);
					if (UserWidget != nullptr) {
						UWidget* Widget = nullptr;
						Widget=UserWidget->GetWidgetFromName(FName(TEXT("[Canvas Panel]")));
						if (Widget != nullptr) {
						}
						Widget = UserWidget->GetWidgetFromName(FName(TEXT("Image_81")));
						//
						if (Widget != nullptr) {
							if (Widget->IsA(UImage::StaticClass())) {
								UImage* Image = Cast<UImage>(Widget);
								UMaterialInstanceDynamic* mid = Image->GetDynamicMaterial();
								mid->SetTextureParameterValue(TEXT("Texture"), MediaTexture);
							}
						}
					}
					continue;
				}
			}
		}
	}
}

void AMediaActor::Bind(UMaterialInterface* MaterialInterface) {
	UObject* Outer = MaterialInterface->GetOuter();
	UMaterialInstanceDynamic* mid=UKismetMaterialLibrary::CreateDynamicMaterialInstance(Outer, MaterialInterface);
	UStaticMeshComponent* StaticMeshComponent = StaticCast<UStaticMeshComponent*>(Outer);
	StaticMeshComponent->SetMaterial(0, mid);
	mid->SetTextureParameterValue(TEXT("Texture"), MediaTexture);
}

// Called every frame
void AMediaActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
