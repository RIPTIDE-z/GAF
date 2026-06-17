#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

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

USTRUCT(BlueprintType)
struct GAFINPUT_API FGAFLookInputSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Look")
	float MousePitchSensitivity{ 1.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Look")
	float MouseYawSensitivity{ 1.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Look")
	float GamepadPitchRate{ 90.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Look")
	float GamepadYawRate{ 240.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Look")
	bool bInvertPitch{ false };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Look")
	bool bInvertYaw{ false };
};

UCLASS(BlueprintType, Const)
class GAFINPUT_API UGAFInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UGAFInputConfig(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "GAF|Pawn")
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	UFUNCTION(BlueprintCallable, Category = "GAF|Pawn")
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", Meta = (TitleProperty = "InputMappingContext"))
	TObjectPtr<UInputMappingContext> InputMappingContext;

	// 手动绑定的输入集合
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", Meta = (TitleProperty = "InputAction"))
	TArray<FGAFInputAction> NativeInputActions;

	// 可以动态绑定的能力输入
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", Meta = (TitleProperty = "InputAction"))
	TArray<FGAFInputAction> AbilityInputActions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", Meta = (TitleProperty = "LookSettings"))
	FGAFLookInputSettings LookSettings;
};
