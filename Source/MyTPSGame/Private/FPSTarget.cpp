// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTPSGame/Public/FPSTarget.h"
#include "MyTPSGame/MyTPSGameCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/GameState.h"
#include "Kismet/GameplayStatics.h"
#include "MyTPSGame/Public/MyGameStateBase.h"
// Sets default values
AFPSTarget::AFPSTarget()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = MeshComp;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("ActorComp"));
	BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	//ActorComp->SetCollisionResponseToChannel();
	BoxComp->SetupAttachment(MeshComp);

}
void AFPSTarget::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AFPSTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFPSTarget::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved,
						FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	//MyComp->GetFName()
	//FName name = this->NamePrivate;
	if (Cast<AMyProjectile>(Other)&&Cast<AMyProjectile>(Other)->WeaponType)
	{
		float distance = HitLocation.Distance(HitLocation, MyComp->GetComponentLocation());
		AMyGameStateBase *MyGameStateBase = Cast<AMyGameStateBase>(GetWorld()->GetGameState());
		switch (static_cast<int8>(distance / 10))
		{
		case 0:
			if(distance<=2)
				FPoints = 10;//Cast<AFPSCharacter>(Other)->
			else if (distance<=6)
				FPoints = 8;
			else
				FPoints = 7;
			break;
		case 1:
			FPoints = 6;
			break;
		case 2:
			FPoints = 4;
			break;
		case 3:
			FPoints = 2;
			break;
		case 4:
			if (distance>45)
				FPoints = 0;
			else
				FPoints = 1;
			
			break;
		case 5:
			FPoints = 0;
			break;
		case 6:
			FPoints = 0;
		}
		MyGameStateBase->FPoints = FPoints;
		MyGameStateBase->FPointsSum += FPoints;
		//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, std::to_string(distance);
		UE_LOG(LogTemp, Warning, TEXT("Test, %f, point, %d, id"), distance, FPoints);
		//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red,
		//								FString("Hit&&&&&&&&&&&&&&&& ") + HitLocation.ToString());
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red,
										FString("Location"));
		
		//Other->Destroy();
		
	}
}