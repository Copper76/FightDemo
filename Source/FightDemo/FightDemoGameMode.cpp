// Copyright Epic Games, Inc. All Rights Reserved.

#include "FightDemoGameMode.h"
#include "FightDemoCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFightDemoGameMode::AFightDemoGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
