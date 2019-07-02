// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "UnrealPlusGameMode.h"
#include "UnrealPlusCharacter.h"
#include "UObject/ConstructorHelpers.h"

AUnrealPlusGameMode::AUnrealPlusGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Game/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
