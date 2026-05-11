#pragma once

#include "Animation/AnimInstance.h"

#include "GAFALSLinkedAnimInstance.generated.h"

class AGAFALSCharacterCore;

UCLASS()
class GAFALS_API UGAFALSLinkedAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TObjectPtr<AGAFALSCharacterCore> Character;
};