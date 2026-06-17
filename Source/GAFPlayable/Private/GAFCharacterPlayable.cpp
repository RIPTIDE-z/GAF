#include "GAFCharacterPlayable.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GAFGamePlayTag.h"
#include "GAFInputBindingHelpers.h"
#include "GAFInputConfig.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Math/RotationMatrix.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFCharacterPlayable)

AGAFCharacterPlayable::AGAFCharacterPlayable()
{
}

// Update input mapping when the pawn changes controller
// Controller变动时绑定IMC
void AGAFCharacterPlayable::NotifyControllerChanged()
{
	if (IsValid(InputConfig) && IsValid(InputConfig->InputMappingContext.Get()))
	{
		// Remove mapping from the previous player controller
		// 移除旧的IMC
		const auto* PreviousPlayerController{ Cast<APlayerController>(PreviousController) };
		if (IsValid(PreviousPlayerController))
		{
			auto* EnhancedInputSubSystem{ ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PreviousPlayerController->GetLocalPlayer()) };
			if (IsValid(EnhancedInputSubSystem))
			{
				EnhancedInputSubSystem->RemoveMappingContext(InputConfig->InputMappingContext.Get());
			}
		}

		auto* NewPlayerController{ Cast<APlayerController>(GetController()) };
		if (IsValid(NewPlayerController))
		{
			auto* EnhancedInputSubsystem{ ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(NewPlayerController->GetLocalPlayer()) };
			if (IsValid(EnhancedInputSubsystem))
			{
				FModifyContextOptions Options;
				// Notify user settings when adding the mapping
				Options.bNotifyUserSettings = true;

				EnhancedInputSubsystem->AddMappingContext(InputConfig->InputMappingContext.Get(), 0, Options);
			}
		}
	}

	Super::NotifyControllerChanged();
}

// Bind concrete input actions
// 绑定Action
void AGAFCharacterPlayable::SetupPlayerInputComponent(UInputComponent* Input)
{
	Super::SetupPlayerInputComponent(Input);

	auto* EnhancedInput{ Cast<UEnhancedInputComponent>(Input) };
	if (!IsValid(EnhancedInput))
	{
		return;
	}

	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered,
	                           this, &ThisClass::Input_OnLookMouse, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Look_Mouse, ETriggerEvent::Canceled,
	                           this, &ThisClass::Input_OnLookMouse, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Look_Gamepad, ETriggerEvent::Triggered,
	                           this, &ThisClass::Input_OnLook, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Look_Gamepad, ETriggerEvent::Canceled,
	                           this, &ThisClass::Input_OnLook, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Move, ETriggerEvent::Triggered,
	                           this, &ThisClass::Input_OnMove, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Move, ETriggerEvent::Canceled,
	                           this, &ThisClass::Input_OnMove, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Move_WorldSpace, ETriggerEvent::Triggered,
	                           this, &ThisClass::Input_OnMoveWorldSpace, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Move_WorldSpace, ETriggerEvent::Canceled,
	                           this, &ThisClass::Input_OnMoveWorldSpace, false);
}

// Apply mouse look input directly
void AGAFCharacterPlayable::Input_OnLookMouse(const FInputActionValue& ActionValue)
{
	const FVector2D Value{ ActionValue.Get<FVector2D>() };
	const FGAFLookInputSettings DefaultLookSettings;
	const auto& LookSettings{ IsValid(InputConfig) ? InputConfig->LookSettings : DefaultLookSettings };
	const auto PitchSign{ LookSettings.bInvertPitch ? -1.0f : 1.0f };
	const auto YawSign{ LookSettings.bInvertYaw ? -1.0f : 1.0f };

	AddControllerPitchInput(Value.Y * PitchSign * LookSettings.MousePitchSensitivity);
	AddControllerYawInput(Value.X * YawSign * LookSettings.MouseYawSensitivity);
}

void AGAFCharacterPlayable::Input_OnLook(const FInputActionValue& ActionValue)
{
	const FVector2D Value{ ActionValue.Get<FVector2D>() };
	const FGAFLookInputSettings DefaultLookSettings;
	const auto& LookSettings{ IsValid(InputConfig) ? InputConfig->LookSettings : DefaultLookSettings };
	const auto PitchSign{ LookSettings.bInvertPitch ? -1.0f : 1.0f };
	const auto YawSign{ LookSettings.bInvertYaw ? -1.0f : 1.0f };

	AddControllerPitchInput(Value.Y * PitchSign * LookSettings.GamepadPitchRate);
	AddControllerYawInput(Value.X * YawSign * LookSettings.GamepadYawRate);
}

void AGAFCharacterPlayable::Input_OnMove(const FInputActionValue& ActionValue)
{
	auto Value{ ActionValue.Get<FVector2D>() };
	if (Value.IsNearlyZero())
	{
		return;
	}

	Value.Normalize();

	const auto ControlRotation{ GetControlRotation() };
	const FRotator YawRotation{ 0.0, ControlRotation.Yaw, 0.0 };
	const auto ForwardDirection{ FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) };
	const auto RightDirection{ FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y) };

	AddMovementInput(RightDirection, Value.X);
	AddMovementInput(ForwardDirection, Value.Y);
}

void AGAFCharacterPlayable::Input_OnMoveWorldSpace(const FInputActionValue& ActionValue)
{
	auto Value{ ActionValue.Get<FVector2D>() };
	if (Value.IsNearlyZero())
	{
		return;
	}

	Value.Normalize();

	AddMovementInput(FVector::RightVector, Value.X);
	AddMovementInput(FVector::ForwardVector, Value.Y);
}
