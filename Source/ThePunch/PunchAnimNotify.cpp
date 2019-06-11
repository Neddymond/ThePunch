// Fill out your copyright notice in the Description page of Project Settings.

#include "PunchAnimNotify.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "ThePunchCharacter.h"

void UPunchAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	GEngine->AddOnScreenDebugMessage(-1, 4.5, FColor::Blue, __FUNCTION__);

	if (MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		AThePunchCharacter* player = Cast<AThePunchCharacter>(MeshComp->GetOwner());

		if (player != NULL && !player->PunchThrowAudioComponent->IsPlaying())
		{
			player->PunchThrowAudioComponent->Play(0.f);
		}
	}
}


