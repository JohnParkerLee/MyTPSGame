// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MyGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class MYTPSGAME_API AMyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	public:
		UPROPERTY(BlueprintReadOnly, Category="Gameplay");
		uint8 FPoints = 0;
		UPROPERTY(BlueprintReadOnly, Category="Gameplay");
		uint8 FPointsSum = 0;
};
