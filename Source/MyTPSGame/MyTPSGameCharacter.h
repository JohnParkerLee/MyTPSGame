// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Public/MyWeapon.h"
#include "MyTPSGameCharacter.generated.h"

UCLASS(config=Game)
class AMyTPSGameCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	/** Gun mesh: 3st person view (seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* GunMeshComponent;
	
	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	USkeletalMeshComponent* Mesh1PComponent;
	//UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	//USkeletalMeshComponent* Mesh;

public:
	AMyTPSGameCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	/** AnimMontage to play each time we fire */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	UAnimSequence* FireAnimation;
	UPROPERTY(BlueprintReadOnly, Category="Gameplay");
	bool bIsCarryingObjective;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;
	UPROPERTY(EditDefaultsOnly, Category="Gameplay")
	TSubclassOf<AMyWeapon> StarterWeaponClass;
	UPROPERTY(EditDefaultsOnly, Category="Gameplay")
	USoundBase* FireSound;
	UPROPERTY(BlueprintReadOnly, Category="Gameplay");
	uint8 FBulletRemain = 10;
	AMyWeapon* EquidWeapon;
	bool bIsCarryWeapon = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Weapon")
	bool WeaponType;

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	void BeginCrouch();

	void EndCrouch();

	void BeginZoom();

	void EndZoom();
	
	/*fire a projectile*/
	UFUNCTION(BlueprintCallable, Category="Control")
	void Fire();

	
	UFUNCTION(BlueprintCallable, Category="Control")
	void SuppleBullet();
	
	FHitResult OutHit;
	bool bWantsToZoom;
	UPROPERTY(EditDefaultsOnly, Category= "Control")
	float ZoomedFOV = 65.0f;;
	/*Begin start FOV*/
	float DefaultFOV;
	UPROPERTY(EditDefaultsOnly, Category= "Control", meta = (ClampMin = 0.1, ClampMax = 100.0))
	float ZoomInterpSpeed = 20.0f;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interfacex
public:
	USkeletalMeshComponent* GetMesh1P() const{return Mesh1PComponent;};
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	virtual FVector GetPawnViewLocation() const override;
};

