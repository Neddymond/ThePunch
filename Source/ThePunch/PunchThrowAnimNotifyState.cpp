// Fill out your copyright notice in the Description page of Project Settings.

#include "PunchThrowAnimNotifyState.h"
#include "Engine/Engine.h"
#include "ThePunchCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UPunchThrowAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	GEngine->AddOnScreenDebugMessage(-1, 4.5, FColor::Orange, TEXT("__FUNCTION__"));

	if (MeshComp != NULL && MeshComp->GetWorld() != NULL)
	{
		AThePunchCharacter* player = Cast<AThePunchCharacter>(MeshComp->GetOwner());

		if (player != NULL && !player->PunchThrowAudioComponent->IsPlaying())
		{
			player->PunchThrowAudioComponent->Play(0.f);
		}
	}
}

void UPunchThrowAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	GEngine->AddOnScreenDebugMessage(-1, 4.5, FColor::Orange, TEXT("__FUNCTION__"));
}




