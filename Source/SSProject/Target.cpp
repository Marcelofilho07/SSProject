// Fill out your copyright notice in the Description page of Project Settings.


#include "Target.h"
#include "HealthComponent.h"
#include "SSProjectGameMode.h"
#include "SSProjectCharacter.h"
#include "Math/UnrealMathUtility.h"


// Sets default values
ATarget::ATarget()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	VisualMesh->SetOnlyOwnerSee(false);
	SetRootComponent(VisualMesh);


	Speed = 500;
	IsHitOnCooldown = 0;
	HitCooldownDuration = 1.5f;
}

// Called when the game starts or when spawned
void ATarget::BeginPlay()
{
	Super::BeginPlay();
	VisualMesh->OnComponentHit.AddDynamic(this, &ATarget::OnHit);
	int32 ArraySize = StaticMeshes.GetAllocatedSize() / StaticMeshes.GetTypeSize();
	int32 FurnitureMeshNumber = FMath::RandRange(0, (ArraySize - 1));
	FString toLog = FString::FromInt(FurnitureMeshNumber);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *toLog);
	if (StaticMeshes.IsValidIndex(FurnitureMeshNumber))
		VisualMesh->SetStaticMesh(StaticMeshes[FurnitureMeshNumber]);
}

// Called every frame
void ATarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetWorld()->GetFirstPlayerController()->GetPawn() && !GetWorld()->GetFirstPlayerController()->GetPawn()->IsPendingKill())
	{
		FVector PlayerLocation = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();

		FVector NewLocation = GetActorLocation();
		FRotator NewRotation = GetActorRotation();

		FVector PlayerDirection = PlayerLocation - NewLocation;
		PlayerDirection.Normalize();
		float RunningTime = GetGameTimeSinceCreation();
		float DeltaHeight = (FMath::Sin(RunningTime + DeltaTime) - FMath::Sin(RunningTime));
		NewLocation.Z += DeltaHeight * 20.0f;       //Scale our height by a factor of 20
		float DeltaRotation = DeltaTime * 20.0f;    //Rotate by 20 degrees per second
		NewRotation.Yaw += DeltaRotation;
		if (Speed == 0)
			Speed = 250;
		SetActorLocationAndRotation((NewLocation + (PlayerDirection * Speed * DeltaTime)), NewRotation, true);
	}

}

void ATarget::ResetHitCooldown()
{
	IsHitOnCooldown = 0;
}


void ATarget::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (IsHitOnCooldown == 0 && OtherActor && OtherActor->CanBeDamaged() && !OtherActor->IsPendingKill())
	{
		if (ASSProjectCharacter* AO = Cast<ASSProjectCharacter>(OtherActor))
		{
			IsHitOnCooldown = 1;
			OtherActor->TakeDamage(DamageDealt, FDamageEvent::FDamageEvent(), DamageInstigator, this);
			GetWorldTimerManager().SetTimer(TimerHandle_HitCooldown, this, &ATarget::ResetHitCooldown, HitCooldownDuration, false);
			if (OtherActor == nullptr)
			{
				if (ASSProjectGameMode* GM = Cast<ASSProjectGameMode>(GetWorld()->GetAuthGameMode()))
				{
					GM->OnTargetHit();
				}
			}
		}
	}
}