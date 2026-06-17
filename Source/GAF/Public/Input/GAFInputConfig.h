#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "GAFInputConfig.generated.h"

class UInputAction;

// 简单外部包装，一个Action对应一个GamePlayTag
USTRUCT(BlueprintType)
struct GAF_API FGAFInputAction
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

// 输入配置资产
UCLASS(BlueprintType, Const)
class GAF_API UGAFInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	UGAFInputConfig(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "GAF|Pawn")
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	UFUNCTION(BlueprintCallable, Category = "GAF|Pawn")
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

public:
	// 手动绑定的输入集合
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FGAFInputAction> NativeInputActions;

	// 可以动态绑定的能力输入
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FGAFInputAction> AbilityInputActions;
};