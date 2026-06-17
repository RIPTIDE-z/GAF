#pragma once

#include "Animation/AnimInstance.h"
#include "Utility/GAFGamePlayTag.h"

#include "GAFAnimInstance.generated.h"

class UGAFLinkedAnimInstance;
class UGAFAnimInstanceSettings;

UCLASS()
class GAF_API UGAFAnimationInstance : public UAnimInstance
{
	GENERATED_BODY()

	friend UGAFLinkedAnimInstance;
};
