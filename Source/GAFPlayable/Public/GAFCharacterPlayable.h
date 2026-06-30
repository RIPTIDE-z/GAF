#pragma once

#include "Character/GAFCharacterCore.h"
#include "Settings/GAFCharacterSettings.h"
#include "Settings/GAFMovementSettings.h"
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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	TObjectPtr<const UGAFCharacterSettings> CharacterSettings;

protected:
	// 组件和蓝图默认值初始化后再应用配置，避免构造CDO时读取空DataAsset。
	virtual void PostInitializeComponents() override;
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
	
	virtual void Input_OnJump(const FInputActionValue& ActionValue);
	
protected:
	// 获取默认角色配置
	const UGAFCharacterSettings& GetDefaultCharacterSettings() const;
	void InitCharacterMovementSettings(UGAFCharacterMovementComponent* CMC, const FGAFMovementSettings& Settings);
};
