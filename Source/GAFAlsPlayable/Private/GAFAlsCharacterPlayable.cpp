#include "GAFAlsCharacterPlayable.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Utility/GAFALSVector.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFAlsCharacterPlayable)

AGAFAlsCharacterPlayable::AGAFAlsCharacterPlayable()
{
}

// 当Pawn的Controller更改时触发
// 负责绑定IMC
void AGAFAlsCharacterPlayable::NotifyControllerChanged()
{
	// 获取之前的controller
	const auto* PreviousPlayerController{ Cast<APlayerController>(PreviousController) };
	if (IsValid(PreviousPlayerController))
	{
		// 获取EnhancedInputLocalPlayerSubsystem
		auto* EnhancedInputSubSystem{ ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PreviousPlayerController->GetLocalPlayer()) };
		if (IsValid(EnhancedInputSubSystem))
		{
			// 移除旧的IMC
			EnhancedInputSubSystem->RemoveMappingContext(InputMappingContext);
		}
	}

	auto* NewPlayerController{ Cast<APlayerController>(GetController()) };
	if (IsValid(NewPlayerController))
	{
		auto* EnhancedInputSubsystem{ ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(NewPlayerController->GetLocalPlayer()) };
		if (IsValid(EnhancedInputSubsystem))
		{
			FModifyContextOptions Options;
			// 开启NotifyUserSettings
			Options.bNotifyUserSettings = true;

			// 注册新的IMC
			EnhancedInputSubsystem->AddMappingContext(InputMappingContext, 0, Options);
		}
	}

	Super::NotifyControllerChanged();
}

// 绑定具体的Action
void AGAFAlsCharacterPlayable::SetupPlayerInputComponent(UInputComponent* Input)
{
	Super::SetupPlayerInputComponent(Input);

	auto* EnhancedInput{ Cast<UEnhancedInputComponent>(Input) };
	if (IsValid(EnhancedInput))
	{
		EnhancedInput->BindAction(LookMouseAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnLookMouse);
		EnhancedInput->BindAction(LookMouseAction, ETriggerEvent::Canceled, this, &ThisClass::Input_OnLookMouse);
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnLook);
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Canceled, this, &ThisClass::Input_OnLook);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnMove);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Canceled, this, &ThisClass::Input_OnMove);
	}
}

// 只传入 Action 的值
void AGAFAlsCharacterPlayable::Input_OnLookMouse(const FInputActionValue& ActionValue)
{
	const FVector2f Value{ ActionValue.Get<FVector2D>() };

	AddControllerPitchInput(Value.Y * LookUpMouseSensitivity);
	AddControllerYawInput(Value.X * LookRightMouseSensitivity);
}

void AGAFAlsCharacterPlayable::Input_OnLook(const FInputActionValue& ActionValue)
{
	const FVector2f Value{ ActionValue.Get<FVector2D>() };

	AddControllerPitchInput(Value.Y * LookUpRate);
	AddControllerYawInput(Value.X * LookRightRate);
}

void AGAFAlsCharacterPlayable::Input_OnMove(const FInputActionValue& ActionValue)
{
	// 将输入值长度限制为1
	const auto Value{ UGAFALSVector::ClampMagnitude012D(ActionValue.Get<FVector2D>()) };

	auto ViewRotation{ GetViewRotation() };

	if (IsValid(GetController()))
	{
		FVector ViewLocation;
		GetController()->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}

	const auto ForwardDirection{ UGAFALSVector::AngleToDirectionXY(UE_REAL_TO_FLOAT(ViewRotation.Yaw)) };
	const auto RightDirection{ UGAFALSVector::PerpendicularCounterClockwiseXY(ForwardDirection) };

	AddMovementInput(ForwardDirection * Value.Y + RightDirection * Value.X);
}