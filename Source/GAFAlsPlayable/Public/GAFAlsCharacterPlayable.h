#pragma once

#include "Character/GAFALSCharacterCore.h"
#include "GAFAlsCharacterPlayable.generated.h"

// 前向声明
// 声明后可将 EnhancedInput 归为Private依赖
struct FInputActionValue;
class UInputMappingContext;
class UInputAction;

// 继承自核心角色类，绑定摄像头、移动组件、输入等
// 是蓝图所要继承的可游玩角色
UCLASS()
class GAFALSPLAYABLE_API AGAFAlsCharacterPlayable : public AGAFALSCharacterCore
{
	GENERATED_BODY()

public:
	AGAFAlsCharacterPlayable();

	// Controller改变时触发
	virtual void NotifyControllerChanged() override;

protected:
	// IMC 资产
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|GAFAls Character Playable|Input Mapping Context")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	// 鼠标视角转动
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|GAFAls Character Playable|Input Action")
	TObjectPtr<UInputAction> LookMouseAction;

	// 手柄视角转动
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|GAFAls Character Playable|Input Action")
	TObjectPtr<UInputAction> LookAction;

	// 移动
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|GAFAls Character Playable|Input Action")
	TObjectPtr<UInputAction> MoveAction;

	// 鼠标视角上下灵敏度
	// 限定最小值为0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|GAFAls Character Playable|Input Settings", Meta = (ClampMin = 0, ForceUnits = "x"))
	float LookUpMouseSensitivity{ 1.0f };

	// 鼠标视角左右灵敏度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|GAFAls Character Playable|Input Settings", Meta = (ClampMin = 0, ForceUnits = "x"))
	float LookRightMouseSensitivity{ 1.0f };

	// 手柄视角上下灵敏度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|GAFAls Character Playable|Input Settings", Meta = (ClampMin = 0, ForceUnits = "deg/s"))
	float LookUpRate{ 90.0f };

	// 手柄视角上下灵敏度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|GAFAls Character Playable|Input Settings", Meta = (ClampMin = 0, ForceUnits = "deg/s"))
	float LookRightRate{ 240.0f };

protected:
	// 绑定按键
	virtual void SetupPlayerInputComponent(UInputComponent* Input) override;

protected:
	// 具体输入动作
	virtual void Input_OnLookMouse(const FInputActionValue& ActionValue);

	virtual void Input_OnLook(const FInputActionValue& ActionValue);

	virtual void Input_OnMove(const FInputActionValue& ActionValue);
};