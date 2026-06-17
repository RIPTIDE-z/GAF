#include "GAFCharacterPlayable.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFCharacterPlayable)

AGAFCharacterPlayable::AGAFCharacterPlayable()
{
}

// Update input mapping when the pawn changes controller
// Controller变动时绑定IMC
void AGAFCharacterPlayable::NotifyControllerChanged()
{
	if (!IsValid(InputConfig) || !IsValid(InputConfig->InputMappingContext))
	{
		UE_LOG(LogTemp, Error, TEXT("InputConfig Invalid"));
		return;
	}
	// Remove mapping from the previous player controller
	// 移除旧的IMC
	const auto* PreviousPlayerController{ Cast<APlayerController>(PreviousController) };
	if (IsValid(PreviousPlayerController))
	{
		auto* EnhancedInputSubSystem{ ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PreviousPlayerController->GetLocalPlayer()) };
		if (IsValid(EnhancedInputSubSystem))
		{
			EnhancedInputSubSystem->RemoveMappingContext(InputConfig->InputMappingContext);
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

			EnhancedInputSubsystem->AddMappingContext(InputConfig->InputMappingContext, 0, Options);
		}
	}

	Super::NotifyControllerChanged();
}

// Bind concrete input actions
// 绑定Action
void AGAFCharacterPlayable::SetupPlayerInputComponent(UInputComponent* Input)
{
	Super::SetupPlayerInputComponent(Input);
	
}

// Apply mouse look input directly
void AGAFCharacterPlayable::Input_OnLookMouse(const FInputActionValue& ActionValue)
{
	const FVector2f Value{ ActionValue.Get<FVector2D>() };

	AddControllerPitchInput(Value.Y * LookUpMouseSensitivity);
	AddControllerYawInput(Value.X * LookRightMouseSensitivity);
}

void AGAFCharacterPlayable::Input_OnLook(const FInputActionValue& ActionValue)
{
	const FVector2f Value{ ActionValue.Get<FVector2D>() };

	AddControllerPitchInput(Value.Y * LookUpRate);
	AddControllerYawInput(Value.X * LookRightRate);
}

void AGAFCharacterPlayable::Input_OnMove(const FInputActionValue& ActionValue)
{

}
