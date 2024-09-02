// Copyright Epic Games, Inc. All Rights Reserved.

#include "FightDemoCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Enemies/Enemy.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AFightDemoCharacter

AFightDemoCharacter::AFightDemoCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	PrimaryActorTick.AddPrerequisite(GetCharacterMovement(), GetCharacterMovement()->PrimaryComponentTick);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AFightDemoCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	LockMarker = GetWorld()->SpawnActor<ALockMarker>(ALockMarker::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
	LockMarker->SetActorHiddenInGame(!bLocking);
	LockMarker->SetActorTickEnabled(bLocking);
}

void AFightDemoCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Call the parent class's EndPlay function
	Super::EndPlay(EndPlayReason);
}

void AFightDemoCharacter::Tick(float DeltaTime)
{
	if (bLocking)
	{
		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(FollowCamera->GetComponentLocation(), LockMarker->GetActorLocation());
		Controller->SetControlRotation(LookAtRotation);
		LockMarker->SetActorRotation(LookAtRotation.GetInverse());
	}
	else
	{
		CurrentEnemy = GetCurrentEnemy();
	}
}

AEnemy* AFightDemoCharacter::GetCurrentEnemy() const
{
	FVector attackDirection = GetVelocity().GetSafeNormal();

	if (attackDirection.IsNearlyZero())
	{
		attackDirection = FollowCamera->GetForwardVector(); //Use Camera direction if player is not moving
	}

	attackDirection.Z = 0.0f;//No vertical checking, enemies should be on a similar platform as player

	FVector startLocation = GetActorLocation();
	FVector endLocation = startLocation + attackDirection * DetectRange;

	float sphereRadius = 50.0f;

	TArray<FHitResult> hitResults;

	// Define collision query parameters
	FCollisionQueryParams params(SCENE_QUERY_STAT(PerformSphereCast), false, this);

	// Perform the sphere cast
	bool bHit = GetWorld()->SweepMultiByChannel(
		hitResults,                      // Array to store hit results
		startLocation,                   // Start location
		endLocation,                     // End location
		FQuat::Identity,                 // Rotation (no rotation for sphere)
		ECC_Visibility,                  // Collision channel (use ECC_Pawn, ECC_WorldStatic, etc.)
		FCollisionShape::MakeSphere(sphereRadius), // Collision shape (sphere with defined radius)
		params                           // Collision query parameters
	);

#if WITH_EDITOR
	DrawDebugLine(GetWorld(), startLocation, endLocation, FColor::Red, false, -1.0f, 0, 5.0f);
#endif

	if (bHit && hitResults.Num() > 0)
	{
		// Sort the results by distance
		hitResults.Sort([](const FHitResult& A, const FHitResult& B) {
			return A.Distance < B.Distance;
			});

		// Process sorted results
		for (const FHitResult& hit : hitResults)
		{
			AActor* hitActor = hit.GetActor();
			if (!hitActor)
			{
				continue;
			}

			//Stop if we hit an enemy
			if (hitActor->IsA(AEnemy::StaticClass()))
			{
#if WITH_EDITOR
				DrawDebugSphere(GetWorld(), hit.ImpactPoint, sphereRadius, 12, FColor::Green, false, -1.0f);
#endif
				return Cast<AEnemy>(hitActor);
			}

			//Stop if we hit a wall
			if (hitActor->GetRootComponent()->Mobility == EComponentMobility::Static)
			{
				break;
			}
		}
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFightDemoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFightDemoCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFightDemoCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LockAction, ETriggerEvent::Triggered, this, &AFightDemoCharacter::ToggleLock);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AFightDemoCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AFightDemoCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr && !bLocking)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AFightDemoCharacter::ToggleLock(const FInputActionValue& Value)
{
	bLocking = !bLocking;

	if (bLocking)
	{
		AEnemy* lockingEnemy = GetBestEnemy();

		if (lockingEnemy)
		{
			CurrentEnemy = lockingEnemy;
			LockMarker->Attach(CurrentEnemy->GetMesh());
		}
		else
		{
			bLocking = false;
		}
	}

	LockMarker->SetActorHiddenInGame(!bLocking);
	LockMarker->SetActorTickEnabled(bLocking);
}

AEnemy* AFightDemoCharacter::GetBestEnemy() const
{
	TArray<AActor*> foundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemy::StaticClass(), foundEnemies);

	//Get Screen centre
	int32 screenWidth, screenHeight;
	PlayerController->GetViewportSize(screenWidth, screenHeight);

	FVector2D screenCentre;
	FVector2D screenPosition;

	// Calculate the centre of the screen
	screenCentre.X = screenWidth / 2.0f;
	screenCentre.Y = screenHeight / 2.0f;

	FVector playerLocation = GetActorLocation();

	AActor* lockingEnemy = nullptr;

	float minDistance = INFINITY;

	for (AActor* enemy : foundEnemies)
	{
		FVector enemyLocation = enemy->GetActorLocation();

		if (FVector::Distance(enemyLocation, playerLocation) > DetectRange)
		{
			continue;
		}

		// Project the world location to screen space
		if (PlayerController->ProjectWorldLocationToScreen(enemyLocation, screenPosition))
		{
			float distanceFromCentre = FVector2D::Distance(screenPosition, screenCentre);

			if (distanceFromCentre < minDistance)
			{
				lockingEnemy = enemy;
				minDistance = distanceFromCentre;
			}
		}
	}

	return Cast<AEnemy>(lockingEnemy);
}
