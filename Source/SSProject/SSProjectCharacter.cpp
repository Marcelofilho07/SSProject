// Copyright Epic Games, Inc. All Rights Reserved.

#include "SSProjectCharacter.h"
#include "SSProjectProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionControllerComponent.h"
#include "UObject/ObjectMacros.h"
#include "SSProjectGameMode.h"
#include "HealthComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ASSProjectCharacter

ASSProjectCharacter::ASSProjectCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.73f, 22.64f, 11.1f));

	Ammo = 10;
	ReloadingTime = 2.f;
	SprintMultiplier = 200.f;
	IsReloading = 0;
	IsSprinting = 0;
}

void ASSProjectCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASSProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASSProjectCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASSProjectCharacter::StopFire);

	// Bind crouch event
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASSProjectCharacter::Crouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASSProjectCharacter::Stand);

	// Bind jump events
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASSProjectCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASSProjectCharacter::StopSprinting);

	// Bind reload event
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASSProjectCharacter::ReloadAnimation);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ASSProjectCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASSProjectCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASSProjectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASSProjectCharacter::LookUpAtRate);
}

void ASSProjectCharacter::StartFire()
{
	if (Ammo > 0 && !IsReloading && !IsSprinting)
	{
		FireShot();

		GetWorldTimerManager().SetTimer(TimerHandle_HandleRefire, this, &ASSProjectCharacter::FireShot, TimeBetweenShots, true);
	}
}

void ASSProjectCharacter::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_HandleRefire);
}

void ASSProjectCharacter::FireShot()
{
	if (Ammo > 0 && !IsReloading && !IsSprinting)
	{
		FHitResult Hit;
		//This verification can be removed since we check for Ammo before calling this funcion
		Ammo--;
		const float WeaponRange = 100000.f;
		const FVector StartTrace = FirstPersonCameraComponent->GetComponentLocation();
		const FVector EndTrace = (FirstPersonCameraComponent->GetForwardVector() * WeaponRange) + StartTrace;

		FCollisionQueryParams QueryParams = FCollisionQueryParams(SCENE_QUERY_STAT(WeaponTrace), false, this);
		//DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Green, false, 5, 0, 3);
		if (GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECC_Visibility, QueryParams))
		{
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FTransform(Hit.ImpactNormal.Rotation(), Hit.ImpactPoint));
			}
			//const FString toLog = Hit.GetActor()->GetName();
			//UE_LOG(LogTemp, Warning, TEXT("%s"), *toLog);
			//if (toLog.Contains("Target"))
			//	Hit.GetActor()->Destroy();
			if (!Hit.GetActor()->IsPendingKill())
			{

				Hit.GetActor()->TakeDamage(110.f, FDamageEvent::FDamageEvent(), this->Controller, this);
				if (Hit.GetActor() == nullptr)
				{
					if (ASSProjectGameMode* GM = Cast<ASSProjectGameMode>(GetWorld()->GetAuthGameMode()))
					{
						GM->OnTargetHit();
					}
				}
			}
		}

		if (MuzzleParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleParticles, FP_Gun->GetSocketTransform(FName("Muzzle")));
		}

		if (FireSound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}

		// try and play a firing animation if specified
		if (FireAnimation != nullptr)
		{
			// Get the animation object for the arms mesh
			UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
			if (AnimInstance != nullptr)
			{
				AnimInstance->Montage_Play(FireAnimation, 1.f);
			}
		}
	}
}


void ASSProjectCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		if (IsSprinting)
		{
			UE_LOG(LogTemp, Warning, TEXT("FLAG IS SPRINTING!"));
			AddMovementInput(GetActorForwardVector(), Value);
		}
		else
			AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ASSProjectCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		if(IsSprinting)
			AddMovementInput(GetActorRightVector(), Value);
		else
			AddMovementInput(GetActorRightVector(), Value);
	}
}


void ASSProjectCharacter::Crouch()
{
	GetCapsuleComponent()->SetCapsuleSize(55.f, 48.0f);
}

void ASSProjectCharacter::Stand()
{
	GetCapsuleComponent()->SetCapsuleSize(55.f, 96.0f);
}

void ASSProjectCharacter::ReloadAnimation()
{
	FP_Gun->SetVisibleFlag(false);
	IsReloading = 1;
	GetWorldTimerManager().SetTimer(TimerHandle_HandleReload, this, &ASSProjectCharacter::Reload, ReloadingTime, false);
}

void ASSProjectCharacter::Reload()
{
	FP_Gun->SetVisibleFlag(true);
	IsReloading = 0;
	Ammo = 30;
}

void ASSProjectCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASSProjectCharacter::Sprint()
{
	UE_LOG(LogTemp, Warning, TEXT("FLAG SPRINT")); 
	IsSprinting = 1;
}

void ASSProjectCharacter::StopSprinting()
{
	IsSprinting = 0;
}

void ASSProjectCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

