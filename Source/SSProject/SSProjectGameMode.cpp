// Copyright Epic Games, Inc. All Rights Reserved.

#include "SSProjectGameMode.h"
#include "SSProjectHUD.h"
#include "SSProjectCharacter.h"
#include "SSProjectGameStateBase.h"
#include "UObject/ConstructorHelpers.h"

ASSProjectGameMode::ASSProjectGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ASSProjectHUD::StaticClass();

	PointsToWin = 1;
}

void ASSProjectGameMode::OnTargetHit()
{
	RunningTime = GetGameTimeSinceCreation();

	if (ASSProjectGameStateBase* GS = Cast<ASSProjectGameStateBase>(GameState))
	{
		GS->Points++;
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, FString::Printf(TEXT("You have %d points!"), GS->Points));
	}

}
