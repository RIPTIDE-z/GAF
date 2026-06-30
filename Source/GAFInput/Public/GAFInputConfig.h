#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GAFInputSettings.h"

#include "GAFInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

// 简单外部包装，一个Action对应一个GamePlayTag
USTRUCT(BlueprintType)
struct GAFINPUT_API FGAFInputAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

UCLASS(BlueprintType, Const)
class GAFINPUT_API UGAFInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UGAFInputConfig(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	UFUNCTION(BlueprintCallable)
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