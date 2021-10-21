// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetSpawner.h"
#include "Target.h"

// Sets default values
ATargetSpawner::ATargetSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	BoxComp->SetBoxExtent(FVector::FVector(10.f, 10.f, 10.f));
	BoxComp->SetupAttachment(RootComponent);
	SpawnTimer = 10.f;
}

// Called when the game starts or when spawned
void ATargetSpawner::BeginPlay()
{
	Super::BeginPlay();
	SpawnLocation = GetActorLocation();
	Rotation = GetActorRotation();
	GetWorldTimerManager().SetTimer(TimerHandle_HandleTargetSpawn, this, &ATargetSpawner::SpawnTarget, SpawnTimer, false);
}


void ATargetSpawner::SpawnTarget()
{
	GetWorld()->SpawnActor<AActor>(ActorToSpawn, SpawnLocation, Rotation);
	if (SpawnTimer > 1)
		SpawnTimer--;
	GetWorldTimerManager().SetTimer(TimerHandle_HandleTargetSpawn, this, &ATargetSpawner::SpawnTarget, SpawnTimer, false);
}

