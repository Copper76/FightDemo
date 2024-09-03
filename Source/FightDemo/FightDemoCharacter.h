// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Misc/LockMarker.h"
#include "Misc/LockComponent.h"

#include "FightDemoCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class AEnemy;

struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UENUM(BlueprintType, meta = (BitFlags, useEnumValuesAsMaskValuesInEditor = "true"))
enum class EPlayerState : uint8
{
	None = 0,
	MOVE = 1 << 0,
	ATTACK = 1 << 1,
	DODGE = 1 << 2,
	COUNTER = 1 << 3,
	HURT = 1 << 4,
};

ENUM_CLASS_FLAGS(EPlayerState);

UCLASS(config=Game)
class AFightDemoCharacter final : public ACharacter
{
	GENERATED_BODY()

public:
	AFightDemoCharacter();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Attack(const FInputActionValue& Value);

	void Counter(const FInputActionValue& Value);

	void ToggleLock(const FInputActionValue& Value);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// To add mapping context
	virtual void BeginPlay();

	virtual void Jump() override;

	virtual void StopJumping() override;

private:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	AEnemy* GetCurrentEnemy() const;

	AEnemy* GetBestEnemy() const;

	void ExecuteAttack();

	void EndAttack();

public:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LockAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CounterAction;

private:
	//Objects
	ALockMarker* LockMarker;

	AEnemy* CurrentTarget;

	AEnemy* CurrentEnemy;

	APlayerController* PlayerController;

private:
	EPlayerState PlayerState = EPlayerState::MOVE;

	EPlayerState AttackableStates = EPlayerState::MOVE | EPlayerState::ATTACK;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Parameters, meta = (AllowPrivateAccess = "true"))
	float DetectRange = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Parameters, meta = (AllowPrivateAccess = "true"))
	float AttackRange = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Parameters, meta = (AllowPrivateAccess = "true"))
	float AttackStopDistance = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Parameters, meta = (AllowPrivateAccess = "true"))
	float EmptyAttackMoveDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Parameters, meta = (AllowPrivateAccess = "true"))
	float TurnToTargetSpeed = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Parameters, meta = (AllowPrivateAccess = "true"))
	float MoveToTargetSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Parameters, meta = (AllowPrivateAccess = "true"))
	float AttackOffset = 100.0f;

	FVector TargetPosition;

	FRotator TargetRotation;

	FVector InputDir;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fight, meta = (AllowPrivateAccess = "true"))
	int MaxCombo = 3;

	int AttackCount = 0;

	int CurrentAttack = 0;

	float AttackTolerance = 10.0f;

private:
	bool bLocking = false;
};

