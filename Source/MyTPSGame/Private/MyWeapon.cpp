// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTPSGame/Public/MyWeapon.h"

#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "MyTPSGame/MyTPSGameCharacter.h"
#include "Particles/ParticleSystemComponent.h"

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
}

// Called when the game starts or when spawned
void AMyWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AMyWeapon::Fire()
{
	// Trace the world , from pawn eyes to crosshair location
	AActor* MyOwner = GetOwner();
	if(MyOwner)
	{

		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();
		
		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);
		FHitResult Hit;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		// more exquisite result
		QueryParams.bTraceComplex = true;

		// Particle Target parameter
		FVector TracerEndPoint = TraceEnd;
		FVector MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
		if(!WeaponType)
		{
			//to-do: to get the socket location or load the new skeleton mesh
			//FVector MuzzleLocation = M
		}
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility,QueryParams))
		{
			//Blocking hit, process damage
			AActor* HitActor = Hit.GetActor();
			// 因为不需要对伤害类型中的变量做出改变，所以使用DamageType使用Tsubclass进行定义
			UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);//???
			if(ImpactEffect){
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
			}
			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			FRotator MuzzleRotation= UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, Hit.Location);
			// spawn the projectile at the muzzle
			GetWorld()->SpawnActor<AMyProjectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, ActorSpawnParams);
			TracerEndPoint = Hit.ImpactPoint;
			
		}
		//DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f,0, 1.0f);
		PlayFireEffects(TracerEndPoint);
		
		
	}
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
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if(TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}
	APawn* MyOwner = Cast<APawn>(GetOwner());
	if(MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if(PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}

void AMyWeapon::NotifyActorBeginOverlap(AActor* OtherActor)
{
	
	AMyTPSGameCharacter* Picker = Cast<AMyTPSGameCharacter>(OtherActor);
	if(Picker&&Picker->EquidWeapon!=this)
	{
		if(!Picker->bIsCarryWeapon)
		{
			AttachToComponent(Picker->GetMesh1P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
			this->SetOwner(Picker);
			Picker->EquidWeapon = this;
			Picker->bIsCarryWeapon = true;
			Picker->WeaponType = this->WeaponType;
		}else
		{
				Picker->EquidWeapon->Destroy();
				this->SetOwner(Picker);
				AttachToComponent(Picker->GetMesh1P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
				Picker->EquidWeapon = this;
				Picker->bIsCarryWeapon = true;
				Picker->WeaponType = this->WeaponType;
		}
		
	}
	
}
