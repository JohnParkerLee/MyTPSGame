// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTPSGame/Public/MyWeapon.h"

#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "MyTPSGame/MyTPSGameCharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "MyTPSGame/MyTPSGame.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AMyWeapon::AMyWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	RootComponent = Sphere;

	//创建静态网格体，并将其附着在根组件上
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StatciMesh"));
	StaticMesh->SetupAttachment(RootComponent);
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	WeaponMesh->CastShadow = false;
	WeaponMesh->SetupAttachment(StaticMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeaponMesh->SetSimulatePhysics(false);

	MuzzleSocketName = "Muzzle";
	TracerTargetName = "Target";

	BaseDamage = 20.0f;

	RateOfFire = 600;

	BulletSpread = 2.0f;

	RateOfFire = 600;

	bIsCarryed = false;

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

// Called when the game starts or when spawned
void AMyWeapon::BeginPlay()
{
	Super::BeginPlay();
	TimeBetweenShots = 60 / RateOfFire;
}

void AMyWeapon::OnRep_HitScanTrace()
{
	// Play cosmetic FX
	PlayFireEffects(HitScanTrace.TraceTo);

	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

// Called every frame
void AMyWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AMyWeapon::Fire()
{
	// In Client
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerFire();
		//return;
	}
	// Trace the world , from pawn eyes to crosshair location
	AActor* MyOwner = GetOwner();
	if (MyOwner&&BulletRemain>0)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		float HalfRad = FMath::DegreesToRadians(BulletSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);
		FHitResult Hit;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		// more exquisite result
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		// Particle Target parameter
		FVector TracerEndPoint = TraceEnd;
		EPhysicalSurface SurfaceType = SurfaceType_Default;
		FVector MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
		if (!WeaponType)
		{
			//to-do: to get the socket location or load the new skeleton mesh
			//FVector MuzzleLocation = M
		}
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			//Blocking hit, process damage
			AActor* HitActor = Hit.GetActor();

			// pervious version
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
			//EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(Hit);
			float ActualDamage = BaseDamage;
			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4.0f;
			}
			// 因为不需要对伤害类型中的变量做出改变，所以使用DamageType使用Tsubclass进行定义
			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit,
			                                   MyOwner->GetInstigatorController(), MyOwner, DamageType); //???

			PlayImpactEffects(SurfaceType, Hit.ImpactPoint);
			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			FRotator MuzzleRotation = UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, Hit.Location);
			// spawn the projectile at the muzzle
			GetWorld()->SpawnActor<AMyProjectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, ActorSpawnParams);
			TracerEndPoint = Hit.ImpactPoint;
		}
		else
		{
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			FRotator MuzzleRotation = UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, TracerEndPoint);
			// spawn the projectile at the muzzle
			GetWorld()->SpawnActor<AMyProjectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, ActorSpawnParams);
		}
		//DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f,0, 1.0f);
		PlayFireEffects(TracerEndPoint);

		if (GetLocalRole() == ROLE_Authority)
		{
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
		}
		BulletRemain--;
		LastFireTime = GetWorld()->TimeSeconds;
	}
}

void AMyWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AMyWeapon::Fire, TimeBetweenShots, true,
	                                FirstDelay);
	//Fire();
}

void AMyWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void AMyWeapon::SetIsCarryed(bool bcarryed)
{
	this->bIsCarryed = bcarryed;
}

void AMyWeapon::PlayFireEffects(FVector TracerEndPoint)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, WeaponMesh, MuzzleSocketName);
	}
	if (TracerEffect)
	{
		FVector MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}
	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC)
		{
			PC->ClientStartCameraShake(FireCamShake);
			//pervious version api, about warning::MyWeapon.cpp(147): [C4996] 'APlayerController::ClientPlayCameraShake':
			//Please use ClientStartCameraShake Please update your code to the new API before upgrading to the next release, otherwise your project will no longer compile.
			//PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}

void AMyWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}
	if (SelectedEffect)
	{
		FVector MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

void AMyWeapon::ServerFire_Implementation()
{
	Fire();
}

bool AMyWeapon::ServerFire_Validate()
{
	return true;
}

void AMyWeapon::NotifyActorBeginOverlap(AActor* OtherActor)
{
	AMyTPSGameCharacter* Picker = Cast<AMyTPSGameCharacter>(OtherActor);
	if (Picker && Picker->EquidWeapon != this && !bIsCarryed)
	{
		if (!Picker->bIsCarryWeapon)
		{
			AttachToComponent(Picker->GetMesh1P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			                  "GripPoint");
			this->SetOwner(Picker);
			Picker->EquidWeapon = this;
			Picker->bIsCarryWeapon = true;
			Picker->WeaponType = this->WeaponType;
		}
		else
		{
			Picker->EquidWeapon->Destroy();
			this->SetOwner(Picker);
			this->bIsCarryed = true;
			AttachToComponent(Picker->GetMesh1P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			                  "GripPoint");
			Picker->EquidWeapon = this;
			Picker->bIsCarryWeapon = true;
			Picker->WeaponType = this->WeaponType;
		}
	}
}

void AMyWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AMyWeapon, HitScanTrace, COND_SkipOwner);
}
