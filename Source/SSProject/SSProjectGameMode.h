// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SSProjectGameMode.generated.h"

UCLASS(minimalapi)
class ASSProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASSProjectGameMode();

	void OnTargetHit();

	int32 PointsToWin;

	float RunningTime;
};



