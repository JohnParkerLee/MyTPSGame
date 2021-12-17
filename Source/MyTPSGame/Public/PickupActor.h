// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PowerupActor.h"
#include "GameFramework/Actor.h"
#include "PickupActor.generated.h"

class USphereComponent;
class UDecalComponent;
UCLASS()
class MYTPSGAME_API APickupActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category="Components")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, Category="Components")
	UDecalComponent* DecalComp;

	UPROPERTY(EditInstanceOnly, Category="PickupActor")
	TSubclassOf<APowerupActor> PowerUpClass;

	APowerupActor* PowerUpInstance;
	UPROPERTY(EditInstanceOnly, Category="PickupActor")
	float CoolDownDuration;

	FTimerHandle TimerHandle_RespawnTimer;

	void Respawn();
public:
	// Sets default values for this actor's properties
	APickupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	// Called every frame
	//virtual void Tick(float DeltaTime) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
