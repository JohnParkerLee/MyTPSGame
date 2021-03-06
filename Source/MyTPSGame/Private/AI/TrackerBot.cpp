// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TrackerBot.h"

#include "DrawDebugHelpers.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
//#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Kismet/GameplayStatics.h"
#include "MyTPSGame/MyTPSGameCharacter.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
// Sets default values
ATrackerBot::ATrackerBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ATrackerBot::HandleTakeDamage);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);


	bUsevelocityChange = false;
	bStartedSelfDestruction = false;
	MovementForce = 2000.0f;
	RequiredDistanceToTarget = 100.0f;

	ExplosionDamage = 40.0f;
	ExplosionRadius = 200.0f;

	SelfDamageInterval = 0.25;

	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ATrackerBot::BeginPlay()
{
	Super::BeginPlay();
	if (GetLocalRole() == ROLE_Authority)
	{
		//Find Initial Move-to
		NextPathPoint = GetNextPathPoint();
	}


	//OnActorBeginOverlap.AddDynamic(this, NotifyActorBeginOverlap());
}

FVector ATrackerBot::GetNextPathPoint()
{
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);
	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);
	if ( NavPath && NavPath->PathPoints.Num() > 1)
	{
		//Return next point in the path
		return NavPath->PathPoints[1];
	}

	//return fail to find path
	return GetActorLocation();
}

void ATrackerBot::HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float
                                   HealthDelta, const class UDamageType* DamageType,
                                   class AController* InstigatedBy, AActor* DamageCauser)
{
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	if (MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());
	// Explode on hitpoint == 0
	if (bExploded)
	{
		return;
	}
	if (Health <= 0.0f)
	{
		SelfDestruct();
	}
}

void ATrackerBot::SelfDestruct()
{
	bExploded = true;
	OnRep_Exploded();
	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if(GetLocalRole()==ROLE_Authority)
	{
		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(this);

		//Apply damage
		UGameplayStatics::ApplyRadialDamage(GetWorld(), ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr,
											IgnoreActors, this,
											GetInstigatorController(), true, ECollisionChannel::ECC_Visibility);

		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1.0f);
		UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());
		//Delete actor later
		//Destroy();
		SetLifeSpan(2.0f);
	}
	
}

void ATrackerBot::OnRep_Exploded()
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
}

void ATrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

// Called every frame
void ATrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() == ROLE_Authority&&!bExploded)
	{
		float DistanceToTarge = (GetActorLocation() - NextPathPoint).Size();
		if (DistanceToTarge <= RequiredDistanceToTarget)
		{
			NextPathPoint = GetNextPathPoint();
			//DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached");
		}
		else
		{
			//Keep moving towards next points
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();
			ForceDirection *= MovementForce;
			MeshComp->AddForce(ForceDirection, NAME_None, bUsevelocityChange);
			//DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32,
			//                          FColor::Yellow, false, 0.0f, 0, 1.0f);
		}
		DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.0f, 1.0f);
	}
	
}


void ATrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	if (!bStartedSelfDestruction && !bExploded)
	{
		AMyTPSGameCharacter* PlayerPawn = Cast<AMyTPSGameCharacter>(OtherActor);
		if (PlayerPawn)
		{
			// when overlapped with as player
			if(GetLocalRole()==ROLE_Authority)
			{
				// Start Self Destory
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ATrackerBot::DamageSelf, SelfDamageInterval,
												true, 0.0f);
			}

			bStartedSelfDestruction = true;

			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
		}
	}
}

// Called to bind functionality to input
/*void ATrackerBot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}*/
void ATrackerBot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATrackerBot, bExploded);
	DOREPLIFETIME(ATrackerBot, bStartedSelfDestruction);
}
