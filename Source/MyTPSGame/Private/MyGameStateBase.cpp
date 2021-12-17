// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameStateBase.h"

#include "Net/UnrealNetwork.h"

void AMyGameStateBase::OnRep_WaveState(EWaveState OldState)
{
	WaveStateChanged(WaveState, OldState);
}

void AMyGameStateBase::SetWaveState(EWaveState NewState)
{
	if(GetLocalRole()== ROLE_Authority)
	{
		EWaveState OldState = WaveState;
		WaveState = NewState;
		// Call on Server
		OnRep_WaveState(OldState);
	}
}

void AMyGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMyGameStateBase, WaveState);
}
