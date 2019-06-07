// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ThePunchCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Sound/SoundCue.h"

//////////////////////////////////////////////////////////////////////////
// AThePunchCharacter

AThePunchCharacter::AThePunchCharacter()
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
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	// load Melee Fist Attack Animation montage
	static ConstructorHelpers::FObjectFinder<UAnimMontage>MeleeFistAttackMontageObject(TEXT("AnimMontage'/Game/Resources/Animations/Melee_Fist_Attack.Melee_Fist_Attack'"));
	if (MeleeFistAttackMontageObject.Succeeded())
	{
		MeleeFistAttackMontage = MeleeFistAttackMontageObject.Object;
	}

	// load Melee Attack Data Table 
	static ConstructorHelpers::FObjectFinder<UDataTable>PlayerAttackMontageDataObject(TEXT("DataTable'/Game/Resources/DataTables/PlayerAttackMontageDataTable.PlayerAttackMontageDataTable'"));
	if (PlayerAttackMontageDataObject.Succeeded())
	{
		PlayerAttackDataTable = PlayerAttackMontageDataObject.Object;
	}

	// Find our Sound cue
	static ConstructorHelpers::FObjectFinder<USoundCue> PunchSoundCueObject(TEXT("SoundCue'/Game/Resources/Audio/PunchSoundCue.PunchSoundCue'"));
	
	// if successful, pass it to punchSoundCue
	if (PunchSoundCueObject.Succeeded())
	{
		PunchSoundCue = PunchSoundCueObject.Object;

		//create a component(Audio component) called "PunchAudioComponent"
		PunchAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("PunchAudioComponent"));
		
		// Attach to Root component of the character
		PunchAudioComponent->SetupAttachment(RootComponent);
	}

	// create a Component(collision box) called "RightMeleeCollisionBox" 
	RightMeleeCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightMeleeCollisionBox"));

	// create a Component(collision box) called "LeftFistCollisionBox" 
	LeftMeleeCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftMeleeCollisionBox"));

	// Attach the collision box to the Root component of our character blueprint
	RightMeleeCollisionBox->SetupAttachment(RootComponent);
	LeftMeleeCollisionBox->SetupAttachment(RootComponent);

	// make the collision box visible in the editor
	RightMeleeCollisionBox->SetHiddenInGame(false);
	LeftMeleeCollisionBox->SetHiddenInGame(false);

	//Set the initial profile name of the collision box
	RightMeleeCollisionBox->SetCollisionProfileName(MeleeCollisionProfile.Disabled);
	LeftMeleeCollisionBox->SetCollisionProfileName(MeleeCollisionProfile.Disabled);

	// Turn off Hit Events on collision
	LeftMeleeCollisionBox->SetNotifyRigidBodyCollision(false);
	RightMeleeCollisionBox->SetNotifyRigidBodyCollision(false);
}

void AThePunchCharacter::BeginPlay()
{
	Super::BeginPlay();

	// attach collision components to sockets based on transformations definition
	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);

	// Attach these components to the named sockets
	LeftMeleeCollisionBox->AttachToComponent(GetMesh(),  AttachmentRules, "fist_l_collision");
	RightMeleeCollisionBox->AttachToComponent(GetMesh(), AttachmentRules, "fist_r_collision");

	LeftMeleeCollisionBox->OnComponentHit.AddDynamic(this, &AThePunchCharacter::OnAttackHit);
	RightMeleeCollisionBox->OnComponentHit.AddDynamic(this, &AThePunchCharacter::OnAttackHit);

	// if PunchAudioComponent and PunchSoundCue is not null
	if (PunchAudioComponent && PunchSoundCue)
	{
		// Attach PunchSoundCue to the PunchAudioComponent
		PunchAudioComponent->SetSound(PunchSoundCue);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AThePunchCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AThePunchCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AThePunchCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AThePunchCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AThePunchCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AThePunchCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AThePunchCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AThePunchCharacter::OnResetVR);

	// Attack Functionality
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AThePunchCharacter::AttackInput);
	PlayerInputComponent->BindAction("Attack", IE_Released, this, &AThePunchCharacter::AttackEnd);
}

void AThePunchCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AThePunchCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AThePunchCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AThePunchCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AThePunchCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AThePunchCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AThePunchCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
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

/// Triggers attack animation based on user input
void AThePunchCharacter::AttackInput()
{
	if (PlayerAttackDataTable)
	{
		static const FString ContextString(TEXT("Player Attack Montage Context"));
		FPlayerAttackMontage* AttackMontage = PlayerAttackDataTable->FindRow<FPlayerAttackMontage>(FName(TEXT("Punch_1")), ContextString, true);

		if (AttackMontage)
		{
			// print this function on the screen, depending on the enum value
			Log(ELogLevel::INFO, __FUNCTION__);

			//generate a random number between 1 and whatever is defined in the data table for this montage
			int MontageSectionIndex = rand() % AttackMontage->AnimSectionCount + 1;

			// Parse the random integer selected to string; concatenate with "start_" to get a name of an animation
			FString MontageSection = "start_" + FString::FromInt(MontageSectionIndex);

			// play random animation selected 
			PlayAnimMontage(AttackMontage->Montage, 1.0f, FName(*MontageSection));
		}
	}
}

void AThePunchCharacter::AttackStart()
{
	// print this function on the screen, depending on the enum value
	Log(ELogLevel::INFO, __FUNCTION__);

	// set the profile name of the collision box when the attack animation starts
	LeftMeleeCollisionBox->SetCollisionProfileName(MeleeCollisionProfile.Enabled);
	RightMeleeCollisionBox->SetCollisionProfileName(MeleeCollisionProfile.Enabled);

	// Generate Hit Events on collision
	LeftMeleeCollisionBox->SetNotifyRigidBodyCollision(true);
	RightMeleeCollisionBox->SetNotifyRigidBodyCollision(true);
}

// Stop Attack Animation
void AThePunchCharacter::AttackEnd()
{
	Log(ELogLevel::INFO, __FUNCTION__);

	// Reset the profile name of the collision box when the attack animation ends
	LeftMeleeCollisionBox->SetCollisionProfileName(MeleeCollisionProfile.Disabled);
	LeftMeleeCollisionBox->SetCollisionProfileName(MeleeCollisionProfile.Disabled);

	// Turn off Hit Events on collision
	LeftMeleeCollisionBox->SetNotifyRigidBodyCollision(false);
	RightMeleeCollisionBox->SetNotifyRigidBodyCollision(false);
}

void AThePunchCharacter::OnAttackHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Log(ELogLevel::WARNING, Hit.GetActor()->GetName());

	// if PunchAudioComponent is not null and if Audio is not playing
	if (PunchAudioComponent && !PunchAudioComponent->IsPlaying())
	{
		// Default pitch = 1; 
		//Set random pitch btw 1 and 1.3
		PunchAudioComponent->SetPitchMultiplier(FMath::RandRange(1.0f, 1.3f));
		// play Audio
		PunchAudioComponent->Play(0.f);
	}
}
void AThePunchCharacter::Log(ELogLevel LogLevel, FString Message)
{
	Log(LogLevel, Message, ELogOutput::ALL);
}

void AThePunchCharacter::Log(ELogLevel LogLevel, FString Message, ELogOutput LogOutput)
{
	// only print when screen is selected, and the GEngine is available
	if ((LogOutput == ELogOutput::ALL || LogOutput == ELogOutput::SCREEN) && GEngine)
	{
		//default color
		FColor LogColor = FColor::Cyan;

		//flip the color based on the type
		switch (LogLevel)
		{
		case ELogLevel::TRACE:
			LogColor = FColor::Green;
			break;
		case ELogLevel::DEBUG:
			LogColor = FColor::Cyan;
			break;
		case ELogLevel::INFO:
			LogColor = FColor::White;
			break;
		case ELogLevel::WARNING:
			LogColor = FColor::Yellow;
			break;
		case ELogLevel::ERROR:
			LogColor = FColor::Red;
			break;
		default:
			break;
		}

		//print the message and leave it on the screen (4.5 controls the duration)
		GEngine->AddOnScreenDebugMessage(-1, 4.5, LogColor, Message);
	}

	//flip the message type based on the error level
	if (LogOutput == ELogOutput::ALL || LogOutput == ELogOutput::OUTPUT_LOG)
	{
		switch (LogLevel)
		{
		case ELogLevel::TRACE:
			UE_LOG(LogTemp, VeryVerbose, TEXT("%s"), *Message)
			break;
		case ELogLevel::DEBUG:
			UE_LOG(LogTemp, Verbose, TEXT("%s"), *Message)
			break;
		case ELogLevel::INFO:
			UE_LOG(LogTemp, Log, TEXT("%s"), *Message)
			break;
		case ELogLevel::WARNING:
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Message)
			break;
		case ELogLevel::ERROR:
			UE_LOG(LogTemp, Error, TEXT("%s"), *Message)
			break;
		default:
			UE_LOG(LogTemp, Log, TEXT("%s"), *Message)
			break;
		}
	}
	
}
