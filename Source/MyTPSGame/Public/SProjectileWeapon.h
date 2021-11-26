// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyWeapon.h"
#include "SProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class MYTPSGAME_API ASProjectileWeapon : public AMyWeapon
{
	GENERATED_BODY()

	protected:

	virtual void Fire() override;

	/*UPROPERTY(EditDefaultsOnly,Category="ProjectileWeapon")
	AMyProjectile* ProjectileClass;*/
	
};
