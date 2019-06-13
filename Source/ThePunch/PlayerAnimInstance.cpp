// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerAnimInstance.h"
#include "ThePunchCharacter.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Engine/Engine.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
	IsInAir = false;
	IsAnimationBlended = true;
	Speed = 0.f;
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Cache pawn for later use
	Owner = TryGetPawnOwner();
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// ensure that Owner is valid
	if (!Owner)
	{
		return;
	}

	if (Owner->IsA(AThePunchCharacter::StaticClass()))
	{
		// Cast the Owner into the PLayerCharacter
		AThePunchCharacter* PlayerCharacter = Cast<AThePunchCharacter>(Owner);

		// If playerCharacter is valid
		if (PlayerCharacter)
		{
			IsInAir = PlayerCharacter->GetMovementComponent()->IsFalling();
			IsAnimationBlended = PlayerCharacter->GetIsAnimationBlended();
			Speed = PlayerCharacter->GetVelocity().Size();
		}

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, "IsInAir " + FString(IsInAir ? "True" : "False"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, "IsAnimationBlended " + FString(IsAnimationBlended ? "True" : "False"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, "Speed " + FString::SanitizeFloat(Speed));
	}
}


