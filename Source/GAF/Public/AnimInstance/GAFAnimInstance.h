#pragma once

#include "Animation/AnimInstance.h"
#include "Utility/GAFGamePlayTag.h"

#include "GAFAnimInstance.generated.h"

class UGAFLinkedAnimInstance;
class UGAFAnimInstanceSettings;
class AGAFCharacterCore;

UCLASS()
class GAF_API UGAFAnimationInstance : public UAnimInstance
{
	GENERATED_BODY()

	friend UGAFLinkedAnimInstance;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TObjectPtr<AGAFCharacterCore> Character;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag Gait{ GAFGaitTags::Walking };
};