#pragma once

#include "Animation/AnimInstance.h"

#include "GAFLinkedAnimInstance.generated.h"

class AGAFCharacterCore;

UCLASS()
class GAF_API UGAFLinkedAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TObjectPtr<AGAFCharacterCore> Character;
};