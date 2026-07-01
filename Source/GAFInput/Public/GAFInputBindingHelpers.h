#pragma once

#include "EnhancedInputComponent.h"
#include "GAFInputConfig.h"
#include "GAFInputLogChannels.h"
#include "GameplayTagContainer.h"

namespace GAFInput
{
	// 根据GamePlayTag找到Config里的Action进行绑定，不硬性绑定特定Action
	template <class UserClass, typename FuncType>
	void BindNativeAction(UEnhancedInputComponent* EnhancedInput, const UGAFInputConfig* InputConfig, const FGameplayTag& InputTag, const ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, const bool bLogIfNotFound = true)
	{
		if (!IsValid(EnhancedInput))
		{
			UE_LOG(LogGAFInput, Warning,
				TEXT("Failed to bind native input [%s]: EnhancedInputComponent is invalid. Object: [%s]."),
				*InputTag.ToString(),
				*GetNameSafe(Cast<UObject>(Object)));

			return;
		}

		if (!IsValid(InputConfig))
		{
			UE_LOG(LogGAFInput, Warning,
				TEXT("Failed to bind native input [%s]: InputConfig is invalid. Object: [%s], EnhancedInputComponent: [%s]."),
				*InputTag.ToString(),
				*GetNameSafe(Cast<UObject>(Object)),
				*GetNameSafe(EnhancedInput));

			return;
		}

		const UInputAction* InputAction =
		InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound);

		if (!IsValid(InputAction))
		{
			if (bLogIfNotFound)
			{
				UE_LOG(LogGAFInput, Warning,
					TEXT("Failed to bind native input [%s]: no InputAction found in InputConfig [%s]. Object: [%s]."),
					*InputTag.ToString(),
					*GetNameSafe(InputConfig),
					*GetNameSafe(Cast<UObject>(Object)));
			}

			return;
		}

		EnhancedInput->BindAction(InputAction, TriggerEvent, Object, Func);
	}
} // namespace GAFInput
