#include "GAFInputConfig.h"

#include "GAFInputLogChannels.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFInputConfig)

UGAFInputConfig::UGAFInputConfig(const FObjectInitializer& ObjectInitializer)
{
}

// 通过GamePlayTag来找到GAFInputAction
const FGAFInputAction* UGAFInputConfig::FindNativeGAFInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FGAFInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return &Action;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogGAFInput, Error, TEXT("Can't find NativeGAFInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

const UInputAction* UGAFInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FGAFInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogGAFInput, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

const FGAFInputAction* UGAFInputConfig::FindAbilityGAFInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FGAFInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return &Action;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogGAFInput, Error, TEXT("Can't find AbilityGAFInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

const UInputAction* UGAFInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FGAFInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogGAFInput, Error, TEXT("Can't find AbilityInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
