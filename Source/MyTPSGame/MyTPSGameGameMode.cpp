// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyTPSGameGameMode.h"

#include "MyGameStateBase.h"
#include "MyPlayerState.h"
#include "MyTPSGameCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMyTPSGameGameMode::AMyTPSGameGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(
		TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	TimeBetweenWaves = 2.0f;

	GameStateClass = AMyGameStateBase::StaticClass();
	PlayerStateClass = AMyPlayerState::StaticClass();
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;
}


void AMyTPSGameGameMode::StartWave()
{
	WaveCount++;
	NumberOfBotsToSpawn = 2 * WaveCount;

	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &AMyTPSGameGameMode::SpawnBotTImerElapse, 1.0f, true,
	                                0.0f);
	SetWaveState(EWaveState::WaveInProgress);
}

void AMyTPSGameGameMode::SpawnBotTImerElapse()
{
	// Blueprint Function(implement in blueprint)
	SpawnNewBot();

	NumberOfBotsToSpawn--;

	if (NumberOfBotsToSpawn <= 0)
	{
		EndWave();
	}
}

void AMyTPSGameGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
	//PrepareFOrNextWave();
	SetWaveState(EWaveState::WaitingToComplete);
}

void AMyTPSGameGameMode::PrepareFOrNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &AMyTPSGameGameMode::StartWave, TimeBetweenWaves,
	                                false);
	SetWaveState(EWaveState::WaitingToStart);

	RestartDeadPlayers();
}

void AMyTPSGameGameMode::CheckWaveState()
{
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);
	if (NumberOfBotsToSpawn > 0 || bIsPreparingForWave)
	{
		return;
	}
	bool bIsAnyBotAlive = false;
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			continue;
		}
		USHealthComponent* HealthComponent = Cast<USHealthComponent>(
			TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (HealthComponent && HealthComponent->GetHealth())
		{
			bIsAnyBotAlive = true;
			break;
		}
	}
	if (!bIsAnyBotAlive)
	{
		SetWaveState(EWaveState::WaveComplete);
		PrepareFOrNextWave();
	}
}

void AMyTPSGameGameMode::CheckAnyPlayerAlive()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			APawn* MyPawn = PC->GetPawn();
			USHealthComponent* HealthComp = Cast<USHealthComponent>(
				MyPawn->GetComponentByClass(USHealthComponent::StaticClass()));
			if (ensure((HealthComp)) && HealthComp->GetHealth() > 0.0f)
			{
				//someone still alive
				return;
			}
		}
	}
	// No one alive
	GameOver();
}

void AMyTPSGameGameMode::GameOver()
{
	EndWave();
	UE_LOG(LogTemp, Log, TEXT("GAME OVER!"));
	SetWaveState(EWaveState::GameOver);

}

void AMyTPSGameGameMode::SetWaveState(EWaveState NewState)
{
	AMyGameStateBase* GS = GetGameState<AMyGameStateBase>();
	if  (ensureAlways(GS))
	{
		GS->SetWaveState(NewState);
		//GS->OnRep_WaveState(NewState);
	}
}

void AMyTPSGameGameMode::RestartDeadPlayers()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn()==nullptr)
		{
			RestartPlayer(PC);
		}
	}
}

void AMyTPSGameGameMode::StartPlay()
{
	Super::StartPlay();
	PrepareFOrNextWave();
	
}

void AMyTPSGameGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckWaveState();

	CheckAnyPlayerAlive();
}


