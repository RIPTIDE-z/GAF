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

	// 切换相机左右偏好侧，输入触发时翻转 Right <-> Left
	virtual void Input_ChangeCameraSide(const FInputActionValue& ActionValue);

	// 鼠标滚轮切换相机视角风格，滚轮 delta 累加到内部 index 后映射到 CameraStyle
	// index 0=Aim, 1=Explore, 2=Combat，超出范围时循环回绕
	virtual void Input_ChangeCameraDistance(const FInputActionValue& ActionValue);

protected:
	// 相机视角风格索引，接收鼠标滚轮 delta 累加，驱动 CurrentCameraStyle
	int32 CameraStyleIndex{ 1 };
};
