// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerState.h"

void AMyPlayerState::AddScore(float ScoreDelta)
{
	SetScore(GetScore()+ScoreDelta);
	//Score += ScoreDelta;
}
