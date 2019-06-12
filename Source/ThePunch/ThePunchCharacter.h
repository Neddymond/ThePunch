// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"

#include "Engine/DataTable.h"

#include "ThePunchCharacter.generated.h"



USTRUCT(BlueprintType)
struct FPlayerAttackMontage : public FTableRowBase
{
	GENERATED_BODY()

	// Melee Fist Attack Montage
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UAnimMontage* Montage;

	// amount of start sections within our montage
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 AnimSectionCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FString Description;
};

USTRUCT(BlueprintType)
struct FMeleeCollisionProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName Enabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName Disabled;

	// default constructor
	FMeleeCollisionProfile()
	{
		Enabled = FName(TEXT("Weapon"));
		Disabled = FName(TEXT("NoCollision"));
	}
};

UENUM(BlueprintType)
enum class ELogLevel : uint8 
{
	TRACE			UMETA(DisplayName = "Trace"),
	DEBUG			UMETA(DisplayName = "Debug"),
	INFO			UMETA(DisplayName = "Info"),
	WARNING			UMETA(DisplayName = "Warning"),
	ERROR			UMETA(DisplayName = "Error")
};

UENUM(BlueprintType)
enum class ELogOutput : uint8 
{
	ALL				UMETA(DisplayName = "All levels"),
	OUTPUT_LOG		UMETA(DisplayName = "Output log"),
	SCREEN			UMETA(DisplayName = "Screen")
};

UENUM(BlueprintType)
enum class ELineTraceType : uint8
{
	CAMERA_SINGLE	UMETA(DisplayName = "Camera - Single line"),
	PLAYER_SINGLE	UMETA(DisplayName = "Player - Single line"),
	CAMERA_SPREAD	UMETA(DisplayName = "Camera - Spread"),
	PLAYER_SPREAD	UMETA(DisplayName = "Player - SPread")
};

UENUM(BlueprintType)
enum class EAttackType : uint8 
{
	MELEE_FIST			UMETA(DisplayName = "Melee - Fist"),
	MELEE_KICK			UMETA(DisplayName = "Melee - Kick")
};

UCLASS(config=Game)
class AThePunchCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	//melee fist attack montage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* MeleeFistAttackMontage;

	//melee fist data Table
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	class UDataTable* PlayerAttackDataTable;

	// load sound cue
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Audio, meta = (AllowPrivateAccess = "true"))
	class USoundCue* PunchSoundCue;

	// load sound cue
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Audio, meta = (AllowPrivateAccess = "true"))
	class USoundCue* PunchThrowSoundCue;
		
	// Right fist collision box
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collision, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* RightMeleeCollisionBox;

	//Left Fist collision box
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collision, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* LeftMeleeCollisionBox;



public:
	AThePunchCharacter();

	void PunchAttack();
	void KickAttack();

	// Triggers Attack animation based on user input
	void AttackInput(EAttackType AttackType);

	// called when the game begins or when the player is spawned
	virtual void BeginPlay() override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	// Triggered when the player initiates an attack
	void AttackStart();

	// Triggered when the player ends an attack
	void AttackEnd();

	//Triggered when the collision hit event fires between our weapon and enemy entities
	UFUNCTION()
	void OnAttackHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// boolean that tells us if we have to branch oour animation blueprint paths
	UFUNCTION(BlueprintCallable, Category = Animation)
	bool GetIsAnimationBlended();

	/** controls if the keyboard responds to user input **/
	UFUNCTION(BlueprintCallable, Category = Animation)
		void SetIsKeyboardEnabled(bool Enabled);

	/** returns the current attack that the player is perfoming **/
	UFUNCTION(BlueprintCallable, Category = Animation)
	EAttackType GetCurrentAttack();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Line Trace")
		ELineTraceType LineTraceType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Line Trace")
		float LineTraceDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Line Trace")
		float LineTraceSpread;

	void FireLineTrace();

protected:

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

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UAudioComponent* PunchThrowAudioComponent;


private:
	UAudioComponent* PunchAudioComponent;

	// structure for PlayerAttackMontage Data Table
	FPlayerAttackMontage* AttackMontage;

	// Collision Profile object; Enabled = "Weapon", Disabled = "NoCollision"
	FMeleeCollisionProfile MeleeCollisionProfile; 

	// Attack Type object
	EAttackType CurrentAttack;

	bool IsAnimationBlended;

	bool IsKeyboardEnabled;


	/**
	* Log - prints a message to all the log outputs with a specific color
	* @param LogLevel {@see ELogLevel} affects color of log
	* @param FString the message for display
	*/
	void Log(ELogLevel LogLevel, FString Message);

	/**
	* Log - prints a message to all the log outputs with a specific color
	* @param LogLevel {@see ELogLevel} affects color of log
	* @param FString the message for display
	* @param ELogOutput - All, Output Log or Screen
	*/
	void Log(ELogLevel LogLevel, FString Message, ELogOutput LogOutput);
};