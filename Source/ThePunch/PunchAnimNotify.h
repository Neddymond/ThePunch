// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PunchAnimNotify.generated.h"


UCLASS()
class THEPUNCH_API UPunchAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
		virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
