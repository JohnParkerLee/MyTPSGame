// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerupActor.generated.h"

UCLASS()
class MYTPSGAME_API APowerupActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APowerupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Time between powerup or ticks
	UPROPERTY(EditDefaultsOnly, Category="Powerups")
	float PowerupInterval;

	// Total times we apply the powerup effect
	UPROPERTY(EditDefaultsOnly, Category="Powerups")
	int32 TotalNumberOfTicks;

	FTimerHandle TimeHandle_PowerupTick;

	// Total number of ticks
	int32 TicksProcessed;
	
	UFUNCTION()
	void OnTickPowerup();

	UPROPERTY(ReplicatedUsing=OnRep_PowerupActive)
	bool bIsPowerupActive;

	UFUNCTION()
	void OnRep_PowerupActive();

	UFUNCTION(BlueprintImplementableEvent, Category="Powerups")
	void OnPowerupStateChanged(bool bNewIsActive);
public:
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

	void ActivatePowerup(AActor* ActivateFor);
	
	UFUNCTION(BlueprintImplementableEvent, Category="Powerups")
	void OnActivated(AActor* ActivateFor);
	UFUNCTION(BlueprintImplementableEvent, Category="Powerups")
	void OnPowerupTicked();

	UFUNCTION(BlueprintImplementableEvent, Category="Powerups")
	void OnExpired();
};
