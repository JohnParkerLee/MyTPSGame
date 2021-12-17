// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SHealthComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "Sound/SoundCue.h"
#include "TrackerBot.generated.h"

UCLASS()
class MYTPSGAME_API ATrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere, Category="Component")
	UStaticMeshComponent* MeshComp;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;
	
	FVector GetNextPathPoint();
	UFUNCTION()
	void HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float
										HealthDelta, const class UDamageType* DamageType,
										class AController* InstigatedBy, AActor* DamageCauser);
	// Next Point in Navigation path
	FVector NextPathPoint;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float MovementForce;
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	bool bUsevelocityChange;
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float RequiredDistanceToTarget;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TrackerBot")
	UParticleSystem* ExplosionEffect;
	
	
	//Dynamic material to pulse on damage
	UMaterialInstanceDynamic* MatInst;

	void SelfDestruct();
    UPROPERTY(ReplicatedUsing=OnRep_Exploded)
	bool bExploded;
	UPROPERTY(Replicated, BlueprintReadOnly, Category="TrackerBot")
	bool bStartedSelfDestruction;

	UFUNCTION()
	void OnRep_Exploded();
	
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionRadius;
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionDamage;

	float SelfDamageInterval;
	
	FTimerHandle TimerHandle_SelfDamage;

	void DamageSelf();
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundCue* SelfDestructSound;
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundCue* ExplodeSound;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Called to bind functionality to input
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
