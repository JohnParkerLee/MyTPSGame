// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerupActor.h"

#include "Net/UnrealNetwork.h"

// Sets default values
APowerupActor::APowerupActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;
	PowerupInterval = 0.0f;
	TotalNumberOfTicks = 0;
	bReplicates = true;
	bIsPowerupActive = false;
}

// Called when the game starts or when spawned
void APowerupActor::BeginPlay()
{
	Super::BeginPlay();
}

void APowerupActor::OnTickPowerup()
{
	TicksProcessed++;
	OnPowerupTicked();
	if (TotalNumberOfTicks <= TicksProcessed)
	{
		OnExpired();
		bIsPowerupActive = true;
		OnRep_PowerupActive();
		// Delete Timer
		GetWorldTimerManager().ClearTimer(TimeHandle_PowerupTick);
	}
}

void APowerupActor::OnRep_PowerupActive()
{
	OnPowerupStateChanged(bIsPowerupActive);
}

void APowerupActor::ActivatePowerup(AActor* ActivateFor)
{
	OnActivated(ActivateFor);
	bIsPowerupActive = true;
	// 保证在服务器上也进行调用，所以需要手动调用该函数，使得服务器、客户端有同样的效果
	OnRep_PowerupActive();
	if (PowerupInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimeHandle_PowerupTick, this, &APowerupActor::OnTickPowerup, PowerupInterval,
		                                true);
	}
	else
	{
		OnTickPowerup();
	}
}

// Called every frame
/*
void APowerupActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
*/
void APowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APowerupActor, bIsPowerupActive);
}
