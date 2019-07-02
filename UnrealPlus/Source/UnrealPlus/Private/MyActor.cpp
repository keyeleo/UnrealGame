// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActor.h"


// Sets default values
AMyActor::AMyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMyActor::ShowSomething(float s){
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, "Log from GEngine ....");
    UE_LOG(LogTemp, Warning, TEXT("This is a testing statement......%f"), s);
}
