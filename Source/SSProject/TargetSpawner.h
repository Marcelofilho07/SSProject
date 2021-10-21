// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "TargetSpawner.generated.h"

UCLASS()
class SSPROJECT_API ATargetSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATargetSpawner();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UBoxComponent* BoxComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	float SpawnTimer;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FTimerHandle TimerHandle_HandleTargetSpawn;

public:	

	void SpawnTarget();

private:
	FVector SpawnLocation;
	FRotator Rotation;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAcess = "true"))
	TSubclassOf<AActor> ActorToSpawn;
};
