#pragma once

#include "Character/GAFCharacterCore.h"
#include "GAFCharacterPlayable.generated.h"

struct FInputActionValue;
class UInputMappingContext;
class UInputAction;
class UGAFInputConfig;

UCLASS()
class GAFPLAYABLE_API AGAFCharacterPlayable : public AGAFCharacterCore
{
	GENERATED_BODY()

public:
	AGAFCharacterPlayable();

	// Controller改变时触发
	virtual void NotifyControllerChanged() override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Input")
	TObjectPtr<const UGAFInputConfig> InputConfig;

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* Input) override;

protected:
	virtual void Input_OnMove(const FInputActionValue& ActionValue);
	virtual void Input_OnMoveWorldSpace(const FInputActionValue& ActionValue);
	virtual void Input_OnLookGamepad(const FInputActionValue& ActionValue);
	virtual void Input_OnLookMouse(const FInputActionValue& ActionValue);
	virtual void Input_OnWalk(const FInputActionValue& ActionValue);
	virtual void Input_OnSprint(const FInputActionValue& ActionValue);
	virtual void Input_OnCrouch(const FInputActionValue& ActionValue);
	virtual void Input_OnJump(const FInputActionValue& ActionValue);
	virtual void Input_OnAim(const FInputActionValue& ActionValue);
	virtual void Input_OnChangeRotationMode(const FInputActionValue& ActionValue);
};
