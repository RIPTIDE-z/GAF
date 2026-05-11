#pragma once

#include "Animation/AnimInstance.h"
#include "Utility/GAFALSGamePlayTag.h"

#include "GAFALSAnimInstance.generated.h"

class UGAFALSLinkedAnimInstance;
class UGAFALSAnimInstanceSettings;
class AGAFALSCharacterCore;

UCLASS()
class GAFALS_API UGAFALSAnimationInstance : public UAnimInstance
{
	GENERATED_BODY()

	friend UGAFALSLinkedAnimInstance;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TObjectPtr<AGAFALSCharacterCore> Character;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag Gait{ GAFALSGaitTags::Walking };
};