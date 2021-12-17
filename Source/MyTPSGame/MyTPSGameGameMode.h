// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MyGameStateBase.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/GameModeBase.h"
#include "MyTPSGameGameMode.generated.h"

enum class EWaveState : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled,AActor*, VictimActor, AActor*, KillerActor, AController*, KillerController); //killed actor, killer actor

UCLASS(minimalapi)
class AMyTPSGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMyTPSGameGameMode();
protected:
	FTimerHandle TimerHandle_BotSpawner;
	FTimerHandle TimerHandle_NextWaveStart;
	// The number of bots to spawn in single wave
	int32 NumberOfBotsToSpawn;

	UPROPERTY(EditDefaultsOnly, Category="GameMode")
	float TimeBetweenWaves;

int32 WaveCount;
protected:
	// Hook for BP to generate single robot
	UFUNCTION(BlueprintImplementableEvent, Category= "GameMode")
	void SpawnNewBot();

	void SpawnBotTImerElapse();

	//start spawn robot
	void StartWave();

	//stop spawn robot
	void EndWave();

	//set timer for next startwave
	void PrepareFOrNextWave();

	void CheckWaveState();

	void CheckAnyPlayerAlive();

	void GameOver();

	void SetWaveState(EWaveState NewState);

	void RestartDeadPlayers();

public:
	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintAssignable, Category= "GameMode")
	FOnActorKilled OnActorKilled;
};
