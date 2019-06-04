// Fill out your copyright notice in the Description page of Project Settings.

#include "AttackStartNotifyState.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "ThePunchCharacter.h"

///The exact time the attack animation is fired
void UAttackStartNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	GEngine->AddOnScreenDebugMessage(-1, 4.5f, FColor::Magenta, __FUNCTION__);

	// check if the mesh is empty
	if (MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		AThePunchCharacter* player = Cast<AThePunchCharacter>(MeshComp->GetOwner());

		if (player != NULL)
		{
			player->AttackStart();
		}
	}

}

///The exact time the attack animation ended
void UAttackStartNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	GEngine->AddOnScreenDebugMessage(-1, 4.5f, FColor::Magenta, __FUNCTION__);

	if (MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		AThePunchCharacter* player = Cast < AThePunchCharacter>(MeshComp->GetOwner());

		if (player != NULL)
		{
			player->AttackEnd();
		}
	}


}



