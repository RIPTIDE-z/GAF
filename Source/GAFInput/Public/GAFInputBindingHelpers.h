#pragma once

#include "EnhancedInputComponent.h"
#include "GAFInputConfig.h"
#include "GameplayTagContainer.h"

namespace GAFInput
{
	// 根据GamePlayTag找到Config里的Action进行绑定，不硬性绑定特定Action
	template <class UserClass, typename FuncType>
	void BindNativeAction(UEnhancedInputComponent* EnhancedInput, const UGAFInputConfig* InputConfig, const FGameplayTag& InputTag, const ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, const bool bLogIfNotFound = true)
	{
		if (EnhancedInput == nullptr || InputConfig == nullptr)
		{
			return;
		}

		if (const UInputAction* InputAction{ InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound) })
		{
			EnhancedInput->BindAction(InputAction, TriggerEvent, Object, Func);
		}
	}
} // namespace GAFInput
