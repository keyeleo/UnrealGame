// Fill out your copyright notice in the Description page of Project Settings.


#include "MediaActor.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Components/Image.h"
#include "WidgetComponent.h"
#include "UserWidget.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"

AMediaActor::AMediaActor()
:MediaPlayer(nullptr)
,MediaTexture(nullptr){
    PrimaryActorTick.bCanEverTick = true;
}

void AMediaActor::BeginPlay(){
    Super::BeginPlay();
    CreateBind();
}

void AMediaActor::CreateBind() {
    MediaPlayer = NewObject<UMediaPlayer>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UMediaPlayer::StaticClass()));
    
    if (MediaPlayer != nullptr){
        UE_LOG(LogTemp, Log, TEXT("---- Create MediaPlayer %s"), *MediaPlayer->GetName());
        FString AssetName = MediaPlayer->GetName() + TEXT("_Video");
        MediaTexture = NewObject<UMediaTexture>(GetTransientPackage(), UMediaTexture::StaticClass(), FName(*AssetName), RF_Public | RF_Standalone | RF_Transactional);
        
        if (MediaTexture != nullptr){
            UE_LOG(LogTemp, Log, TEXT("---- Create MediaTexture %s"), *MediaTexture->GetName());
            MediaTexture->SetDefaultMediaPlayer(MediaPlayer);
            
            TInlineComponentArray<UActorComponent*> Components;
            GetComponents(Components);
            for (UActorComponent* Component : Components) {
                if (Component->IsA(UStaticMeshComponent::StaticClass())) {
                    BindMesh(StaticCast<UStaticMeshComponent*>(Component));
                    continue;
                }
                
                if (Component->IsA(UWidgetComponent::StaticClass())) {
                    BindWidget(StaticCast<UWidgetComponent*>(Component));
                    continue;
                }
            }
        }
    }
}

void AMediaActor::BindMesh(UStaticMeshComponent* StaticMeshComponent, int MaterialIndex, const FString& TextureParameterName){
    UMaterialInterface* MaterialInterface = StaticMeshComponent->GetMaterial(MaterialIndex);
    UMaterialInstanceDynamic* Mid = UKismetMaterialLibrary::CreateDynamicMaterialInstance(StaticMeshComponent, MaterialInterface);
    if(Mid!=nullptr){
        StaticMeshComponent->SetMaterial(MaterialIndex, Mid);
        Mid->SetTextureParameterValue(FName(*TextureParameterName), MediaTexture);
        UE_LOG(LogTemp, Log, TEXT("Bind Material for %s"), *StaticMeshComponent->GetName());
    }
}

void AMediaActor::BindWidget(UWidgetComponent* WidgetComponent, const FString& ImageName, const FString& TextureParameterName){
    UUserWidget* UserWidget=WidgetComponent->GetUserWidgetObject();
    if (UserWidget != nullptr) {
        UWidget* Widget = UserWidget->GetWidgetFromName(FName(*ImageName));
        if (Widget != nullptr && Widget->IsA(UImage::StaticClass())) {
            UMaterialInstanceDynamic* Mid = Cast<UImage>(Widget)->GetDynamicMaterial();
            if(Mid!=nullptr){
                Mid->SetTextureParameterValue(FName(*TextureParameterName), MediaTexture);
                UE_LOG(LogTemp, Log, TEXT("Bind Material for %s"), *Widget->GetName());
            }
        }
    }
}

void AMediaActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
