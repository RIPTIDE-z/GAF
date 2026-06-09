#pragma once

#include "Character/GAFCharacterCore.h"
#include "GAFCharacterPlayable.generated.h"

// Forward declarations keep EnhancedInput as a private dependency
struct FInputActionValue;
class UInputMappingContext;
class UInputAction;

// Playable character base for blueprint-controlled pawns
UCLASS()
class GAFPLAYABLE_API AGAFCharacterPlayable : public AGAFCharacterCore
{
	GENERATED_BODY()

public:
	AGAFCharacterPlayable();

	// React when this pawn changes controller
	virtual void NotifyControllerChanged() override;

protected:
	// Input mapping context asset
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|GAF Character Playable|Input Mapping Context")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	// Mouse look action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|GAF Character Playable|Input Action")
	TObjectPtr<UInputAction> LookMouseAction;

	// Gamepad look action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|GAF Character Playable|Input Action")
	TObjectPtr<UInputAction> LookAction;

	// Movement action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|GAF Character Playable|Input Action")
	TObjectPtr<UInputAction> MoveAction;

	// Mouse vertical look sensitivity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|GAF Character Playable|Input Settings", Meta = (ClampMin = 0, ForceUnits = "x"))
	float LookUpMouseSensitivity{ 1.0f };

	// Mouse horizontal look sensitivity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|GAF Character Playable|Input Settings", Meta = (ClampMin = 0, ForceUnits = "x"))
	float LookRightMouseSensitivity{ 1.0f };

	// Gamepad vertical look rate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|GAF Character Playable|Input Settings", Meta = (ClampMin = 0, ForceUnits = "deg/s"))
	float LookUpRate{ 90.0f };

	// Gamepad horizontal look rate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|GAF Character Playable|Input Settings", Meta = (ClampMin = 0, ForceUnits = "deg/s"))
	float LookRightRate{ 240.0f };

protected:
	// Bind input actions
	virtual void SetupPlayerInputComponent(UInputComponent* Input) override;

protected:
	// Input action handlers
	virtual void Input_OnLookMouse(const FInputActionValue& ActionValue);

	virtual void Input_OnLook(const FInputActionValue& ActionValue);

	virtual void Input_OnMove(const FInputActionValue& ActionValue);
};
