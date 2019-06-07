// Fill out your copyright notice in the Description page of Project Settings.

#include "PunchThrowAnimNotifyState.h"
#include "Engine/Engine.h"

void UPunchThrowAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	GEngine->AddOnScreenDebugMessage(-1, 4.5, FColor::Orange, TEXT("__FUNCTION__"));
}

void UPunchThrowAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	GEngine->AddOnScreenDebugMessage(-1, 4.5, FColor::Orange, TEXT("__FUNCTION__"));
}




