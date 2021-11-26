// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "FPSTarget.generated.h"

UCLASS()
class MYTPSGAME_API AFPSTarget : public AActor
{
	GENERATED_BODY()
	
	public:	
	// Sets default values for this actor's properties
	AFPSTarget();
	UPROPERTY(BlueprintReadOnly, Category="Gameplay");
	uint8 FPoints = 0;

	protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere, Category="Components");
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category="Components");
	UBoxComponent* BoxComp;
	

	public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
};
