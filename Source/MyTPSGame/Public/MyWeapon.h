// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyProjectile.h"
#include "GameFramework/Actor.h"
#include "MyWeapon.generated.h"
class UDamageType;
class UParticleSystem;

// Contains information of a single hitscan weapon linetrace
USTRUCT()
struct FHitSacnTrace
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceTo;
};

UCLASS()
class MYTPSGAME_API AMyWeapon : public AActor
{
	GENERATED_BODY()

	public:	
	// Sets default values for this actor's properties
	AMyWeapon();
	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	TSubclassOf<AMyProjectile> ProjectileClass;
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* Sphere;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class USkeletalMeshComponent* WeaponMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StaticMesh ")
	class UStaticMeshComponent* StaticMesh;
	//重写重叠函数
	
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
	//声明拾取函数
	void Equip(class AMan* Picker);
	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	FName MuzzleSocketName;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	FName TracerTargetName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	UParticleSystem* FleshImpactEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	UParticleSystem* TracerEffect;
	// WeaponType=0,枪榴弹， WeaponType=1,普通子弹
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	bool WeaponType=1;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void PlayFireEffects(FVector TracerEndPoint);

	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	TSubclassOf<UMatineeCameraShake> FireCamShake;
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	float BaseDamage;
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();
	FTimerHandle TimerHandle_TimeBetweenShots;
	float LastFireTime;
	/*RPM Bullets per min*/
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	float RateOfFire;
	/*Derived from reteoftime*/
	float TimeBetweenShots;
	bool bIsCarryed;


	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitSacnTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable, Category="Weapon")
	virtual void Fire();

	void StartFire();

	void StopFire();

	void SetIsCarryed(bool bCarryed);
};
