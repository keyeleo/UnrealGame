// Fill out your copyright notice in the Description page of Project Settings.


#include "ARobot.h"


// Sets default values
AARobot::AARobot()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AARobot::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AARobot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

