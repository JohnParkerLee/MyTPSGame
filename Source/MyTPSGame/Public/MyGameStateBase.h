// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MyGameStateBase.generated.h"

UENUM(BlueprintType)
enum class EWaveState:uint8
{
	WaitingToStart,

	WaveInProgress,

	//No longer spawning new bots, waiting for players for players to kill remaining bots
	WaitingToComplete,

	GameOver,

	WaveComplete,
};

/**
 * 
 */
UCLASS()
class MYTPSGAME_API AMyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()


protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Gameplay")
	void WaveStateChanged(EWaveState NewState, EWaveState OldState);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_WaveState, Category="Gameplay")
	EWaveState WaveState;
	UFUNCTION()
	void OnRep_WaveState(EWaveState OldState);

public:
	UPROPERTY(BlueprintReadOnly, Category="Gameplay")
	int32 FPoints = 0;
	UPROPERTY(BlueprintReadOnly, Category="Gameplay")
	int32 FPointsSum = 0;

	void SetWaveState(EWaveState NewState);
};
