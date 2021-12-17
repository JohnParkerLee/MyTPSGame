// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTPSGame/Public/SProjectileWeapon.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"


void ASProjectileWeapon::Fire()
{
	// In Client
	if(GetLocalRole() < ROLE_Authority)
	{
		ServerFire();
		//return;
	}
	AActor* MyOwner = GetOwner();
	if(MyOwner&&ProjectileClass)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		FHitResult Hit;
		FVector ShotDirection = EyeRotation.Vector();
		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);
		FVector TracerEndPoint = TraceEnd;
		FVector MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		// more exquisite result
		QueryParams.bTraceComplex = true;
		EPhysicalSurface SurfaceType = SurfaceType_Default;
		//FVector MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
		//FRotator MuzzleRotation = WeaponMesh->GetSocketRotation(MuzzleSocketName);

		//FActorSpawnParameters ActorSpawnParams;
		//ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		 
		//GetWorld()->SpawnActor<AMyProjectile>(ProjectileClass, MuzzleLocation, EyeRotation, ActorSpawnParams);
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility,QueryParams))
		{
			//Blocking hit, process damage
			AActor* HitActor = Hit.GetActor();
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
			// 因为不需要对伤害类型中的变量做出改变，所以使用DamageType使用Tsubclass进行定义
			//UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);//???
			if(DefaultImpactEffect){
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DefaultImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
			}
			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			FRotator MuzzleRotation= UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, Hit.Location);
			// spawn the projectile at the muzzle
			GetWorld()->SpawnActor<AMyProjectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, ActorSpawnParams);
			TracerEndPoint = Hit.ImpactPoint;
			
		}
		else{
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			FRotator MuzzleRotation= UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, TracerEndPoint);
			// spawn the projectile at the muzzle
			GetWorld()->SpawnActor<AMyProjectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, ActorSpawnParams);
		}
		if(GetLocalRole() == ROLE_Authority)
		{
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
		}
	}
}
