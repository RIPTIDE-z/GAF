#pragma once

#include "Character/GAFCharacterCore.h"
#include "GAFCharacterPlayable.generated.h"

struct FInputActionValue;
enum class EGAFTraversalDebugType : uint8;
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
	virtual void HandleInputPressed(FGameplayTag InputTag);
	virtual void HandleInputReleased(FGameplayTag InputTag);

protected:
	virtual void Input_OnMove(const FInputActionValue& ActionValue);
	virtual void Input_OnMoveWorldSpace(const FInputActionValue& ActionValue);
	virtual void Input_OnLookGamepad(const FInputActionValue& ActionValue);
	virtual void Input_OnLookMouse(const FInputActionValue& ActionValue);

	virtual void Input_OnWalkPressed(const FInputActionValue& ActionValue);
	virtual void Input_OnWalkReleased(const FInputActionValue& ActionValue);

	virtual void Input_OnSprintPressed(const FInputActionValue& ActionValue);
	virtual void Input_OnSprintReleased(const FInputActionValue& ActionValue);

	virtual void Input_OnCrouchPressed(const FInputActionValue& ActionValue);
	virtual void Input_OnCrouchReleased(const FInputActionValue& ActionValue);

	virtual void Input_OnAimPressed(const FInputActionValue& ActionValue);
	virtual void Input_OnAimReleased(const FInputActionValue& ActionValue);

	virtual void Input_OnChangeRotationModePressed(const FInputActionValue& ActionValue);
	virtual void Input_OnChangeRotationModeReleased(const FInputActionValue& ActionValue);

	virtual void Input_OnJumpStarted(const FInputActionValue& ActionValue);
	virtual void Input_OnJumpTriggered(const FInputActionValue& ActionValue);
	virtual void Input_OnJumpReleased(const FInputActionValue& ActionValue);
};
