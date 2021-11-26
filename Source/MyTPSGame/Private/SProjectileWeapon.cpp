// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTPSGame/Public/SProjectileWeapon.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


void ASProjectileWeapon::Fire()
{
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
		//FVector MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
		//FRotator MuzzleRotation = WeaponMesh->GetSocketRotation(MuzzleSocketName);

		//FActorSpawnParameters ActorSpawnParams;
		//ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		 
		//GetWorld()->SpawnActor<AMyProjectile>(ProjectileClass, MuzzleLocation, EyeRotation, ActorSpawnParams);
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
		
	}
}
