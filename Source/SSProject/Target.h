// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Target.generated.h"

UCLASS()
class SSPROJECT_API ATarget : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATarget();


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class AController* DamageInstigator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMeshComponent* VisualMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float HitCooldownDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float DamageDealt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TArray<UStaticMesh*> StaticMeshes;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	float IsHitOnCooldown;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ResetHitCooldown();

	FTimerHandle TimerHandle_HitCooldown;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

};
