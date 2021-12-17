// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupActor.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "MyTPSGame/MyTPSGameCharacter.h"


// Sets default values
APickupActor::APickupActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(75.0f);
	RootComponent = SphereComp;
	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	DecalComp->DecalSize = FVector(64, 75, 75);
	DecalComp->SetupAttachment(RootComponent);
	CoolDownDuration = 10.0f;
	bReplicates = true;
}

void APickupActor::Respawn()
{
	if (PowerUpClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PowerUpClass is nullptr in %s. Please update your Blurprint"), *GetName());
		return;
	}
	FActorSpawnParameters SpawnParam;
	SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	PowerUpInstance = GetWorld()->SpawnActor<APowerupActor>(PowerUpClass, GetTransform(), SpawnParam);
}

// Called when the game starts or when spawned
void APickupActor::BeginPlay()
{
	Super::BeginPlay();
	if (GetLocalRole() == ROLE_Authority)
	{
		Respawn();
	}
}

void APickupActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	if (GetLocalRole() == ROLE_Authority)
	{
		if (Cast<AMyTPSGameCharacter>(OtherActor))
		{
			if (PowerUpInstance)
			{
				PowerUpInstance->ActivatePowerup(OtherActor);
				PowerUpInstance = nullptr;

				//Set Timer to respawn
				GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &APickupActor::Respawn,
				                                CoolDownDuration);
			}
		}
	}
}

// Called every frame
/*void APickupActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}*/
