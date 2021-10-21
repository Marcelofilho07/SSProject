// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SSProjectGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class SSPROJECT_API ASSProjectGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	ASSProjectGameStateBase();

	int32 Points;
};
