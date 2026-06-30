#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GAFInputSettings.h"

#include "GAFInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

UENUM(BlueprintType)
enum class EGAFInputStateActivationMode : uint8
{
	// 无，比如Look/Move不需要
	None,
	// 按住
	Hold,
	// 切换
	Toggle
};

// 简单外部包装，一个Action对应一个GamePlayTag
// 同时叠加触发的Action还分按住触发和切换触发
USTRUCT(BlueprintType)
struct GAFINPUT_API FGAFInputAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	FGameplayTag InputTag;

	// None: 普通输入，比如 Move / Look
	// Hold: 按住生效，松开取消，比如按住 Sprint
	// Toggle: 按一下切换开关，比如按一下切换为 Walk
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	EGAFInputStateActivationMode StateActivationMode{ EGAFInputStateActivationMode::None };

	// 这个输入动作要修改的角色输入状态，比如 GAF.InputState.WantsToRun
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input State", Meta = (Categories = "InputState"))
	FGameplayTag InputStateTag;
};

UCLASS(BlueprintType, Const)
class GAFINPUT_API UGAFInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UGAFInputConfig(const FObjectInitializer& ObjectInitializer);

	// 返回包装的Action
	const FGAFInputAction* FindNativeGAFInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	// 返回原Action
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	const FGAFInputAction* FindAbilityGAFInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InputBinding")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	// 手动绑定的输入集合
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InputBinding")
	TArray<FGAFInputAction> NativeInputActions;

	// 可以动态绑定的能力输入
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InputBinding")
	TArray<FGAFInputAction> AbilityInputActions;

	// 输入参数
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (ShowOnlyInnerProperties))
	FGAFInputSettings InputSettings;
};