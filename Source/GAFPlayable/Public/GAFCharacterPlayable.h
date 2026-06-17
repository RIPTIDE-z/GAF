#pragma once

#include "Character/GAFCharacterCore.h"
#include "GAFCharacterPlayable.generated.h"

// Forward declarations keep EnhancedInput as a private dependency
struct FInputActionValue;
class UInputMappingContext;
class UInputAction;
class UGAFInputConfig;

// Playable character base for blueprint-controlled pawns
UCLASS()
class GAFPLAYABLE_API AGAFCharacterPlayable : public AGAFCharacterCore
{
	GENERATED_BODY()

public:
	AGAFCharacterPlayable();

	// React when this pawn changes controller
	virtual void NotifyControllerChanged() override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Input")
	TObjectPtr<const UGAFInputConfig> InputConfig;
	

protected:
	// Bind input actions
	virtual void SetupPlayerInputComponent(UInputComponent* Input) override;

protected:
	// Input action handlers
	virtual void Input_OnLookMouse(const FInputActionValue& ActionValue);

	virtual void Input_OnLook(const FInputActionValue& ActionValue);

	virtual void Input_OnMove(const FInputActionValue& ActionValue);
};
