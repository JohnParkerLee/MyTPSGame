// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyTPSGameCharacter.h"

#include "DrawDebugHelpers.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "MyTPSGame.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Public/MyGameStateBase.h"
#include "Public/MyProjectile.h"
#include "Public/Components/SHealthComponent.h"

//////////////////////////////////////////////////////////////////////////
// AMyTPSGameCharacter

AMyTPSGameCharacter::AMyTPSGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;


	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);
	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	//GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	//FollowCamera->bUsePawnControlRotation = true; // Camera does not rotate relative to arm

	// Create a gun mesh component
	GunMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	GunMeshComponent->CastShadow = false;
	Mesh1PComponent = FindComponentByClass<USkeletalMeshComponent>();
	GunMeshComponent->SetupAttachment(Mesh1PComponent, "GripPoint");

	//Set up HealthComponent
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
}

//////////////////////////////////////////////////////////////////////////
// Input


void AMyTPSGameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMyTPSGameCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AMyTPSGameCharacter::StopFire);
	PlayerInputComponent->BindAction("SuppleBullet", IE_Pressed, this, &AMyTPSGameCharacter::SuppleBullet);
	PlayerInputComponent->BindAxis("MoveForward", this, &AMyTPSGameCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyTPSGameCharacter::MoveRight);
	// Action crounch added
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMyTPSGameCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AMyTPSGameCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &AMyTPSGameCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &AMyTPSGameCharacter::EndZoom);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMyTPSGameCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMyTPSGameCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AMyTPSGameCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AMyTPSGameCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AMyTPSGameCharacter::OnResetVR);
}

FVector AMyTPSGameCharacter::GetPawnViewLocation() const
{
	if (FollowCamera)
	{
		return FollowCamera->GetComponentLocation();
	}
	return Super::GetPawnViewLocation();
}

void AMyTPSGameCharacter::BeginPlay()
{
	Super::BeginPlay();
	DefaultFOV = FollowCamera->FieldOfView;
	HealthComp->OnHealthChanged.AddDynamic(this, &AMyTPSGameCharacter::OnHealthChanged);
	if (GetLocalRole() == ROLE_Authority)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		EquidWeapon = GetWorld()->SpawnActor<AMyWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator,
		                                                SpawnParameters);

		if (EquidWeapon)
		{
			bIsCarryWeapon = true;
			EquidWeapon->SetIsCarryed(bIsCarryWeapon);
			EquidWeapon->SetOwner(this);
			EquidWeapon->AttachToComponent(GetMesh1P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			                               "GripPoint");
			this->WeaponType = EquidWeapon->WeaponType;
			//WeaponType = Cast<AMyProjectile>(EquidWeapon->ProjectileClass)->WeaponType;
		}
	}
	
}

void AMyTPSGameCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;
	float NewFOV = FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaSeconds, ZoomInterpSpeed);
	FollowCamera->SetFieldOfView(NewFOV);
}


void AMyTPSGameCharacter::StartFire()
{
	// try and fire a projectile
	if (FBulletRemain > 0 && bIsCarryWeapon)
	{
		// fire function in weapon
		if (EquidWeapon->ProjectileClass)
		{
			EquidWeapon->StartFire();
			FBulletRemain = EquidWeapon->BulletRemain;
		}
		/* First person fire funciton
		if (EquidWeapon->ProjectileClass)
		{
			// Grabs location from the mesh that must have a socket called "Muzzle" in his skeleton
			FVector MuzzleLocation = EquidWeapon->WeaponMesh->GetSocketLocation("Muzzle");
			// Use controller rotation which is our view direction in first person
			FRotator MuzzleRotation = GetControlRotation();

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			
			// spawn the projectile at the muzzle
			GetWorld()->SpawnActor<AMyProjectile>(EquidWeapon->ProjectileClass, MuzzleLocation, MuzzleRotation, ActorSpawnParams);
		}*/


		/* fire function in character
		if (EquidWeapon->ProjectileClass)
		{
			FCollisionQueryParams QueryParams;
			//QueryParams.AddIgnoredActor(QueryParams);
			QueryParams.AddIgnoredActor(this);
			// more exquisite result
			QueryParams.bTraceComplex = true;
			GetWorld()->LineTraceSingleByChannel(OutHit, GetActorLocation(), GetActorForwardVector() * 10000.f + GetActorLocation(), ECollisionChannel::ECC_Visibility, QueryParams);
			//OutHit.Location
			FRotator MuzzleRotation= UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), OutHit.Location);
			DrawDebugLine(GetWorld(), GetActorLocation(), OutHit.Location, FColor::White, false, 1.0f,0, 1.0f);
			UKismetMathLibrary::MakeTransform(EquidWeapon->GetActorLocation(), MuzzleRotation, FVector(1.0,1.0,1.0));
			// Grabs location from the mesh that must have a socket called "Muzzle" in his skeleton
			FVector MuzzleLocation = EquidWeapon->WeaponMesh->GetSocketLocation("Muzzle");
			// Use controller rotation which is our view direction in first person
			//FRotator MuzzleRotation = GetControlRotation();

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			
			// spawn the projectile at the muzzle
			GetWorld()->SpawnActor<AMyProjectile>(EquidWeapon->ProjectileClass, MuzzleLocation, MuzzleRotation, ActorSpawnParams);
		}
		*/
		// try and play the sound if specified
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}

		// try and play a firing animation if specified
		if (FireAnimation)
		{
			// Get the animation object for the arms mesh
			UAnimInstance* AnimInstance = Mesh1PComponent->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->PlaySlotAnimationAsDynamicMontage(FireAnimation, "fire", 0.0f);
			}
		}
		FBulletRemain -= 1;
	}
	else if (FBulletRemain == 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 100, FColor::White, TEXT("Press Button to restart"), true,
		                                 FVector2D(2, 2));
	}
}

void AMyTPSGameCharacter::StopFire()
{
	if (EquidWeapon)
	{
		EquidWeapon->StopFire();
	}
}


void AMyTPSGameCharacter::SuppleBullet()
{
	FBulletRemain = 100;
	EquidWeapon->BulletRemain = 100;
	AMyGameStateBase* MyGameStateBase = Cast<AMyGameStateBase>(GetWorld()->GetGameState());
	MyGameStateBase->FPoints = 0;
	MyGameStateBase->FPointsSum = 0;
	GEngine->ClearOnScreenDebugMessages();
}

void AMyTPSGameCharacter::OnHealthChanged(USHealthComponent* HealthCompent, float Health, float
                                          HealthDelta, const class UDamageType* DamageType,
                                          class AController* InstigatedBy, AActor* DamageCauser)
{
	if(Health<=0.0f && !bDied)
	{
		//Die animation
		bDied = true;
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//DetachFromControllerPendingDestroy();

		SetLifeSpan(10.0f);
		
	}
}

void AMyTPSGameCharacter::OnResetVR()
{
	// If MyTPSGame is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in MyTPSGame.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AMyTPSGameCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AMyTPSGameCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AMyTPSGameCharacter::BeginCrouch()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red,
	                                 FString("Crouch"));
	Crouch();
}

void AMyTPSGameCharacter::EndCrouch()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red,
	                                 FString("Crouch END"));
	UnCrouch();
}

void AMyTPSGameCharacter::BeginZoom()
{
	bWantsToZoom = true;
}

void AMyTPSGameCharacter::EndZoom()
{
	bWantsToZoom = false;
}

void AMyTPSGameCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMyTPSGameCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMyTPSGameCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMyTPSGameCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AMyTPSGameCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMyTPSGameCharacter, EquidWeapon);
	DOREPLIFETIME(AMyTPSGameCharacter, bDied);
}
