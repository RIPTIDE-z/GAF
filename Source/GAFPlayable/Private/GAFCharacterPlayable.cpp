#include "GAFCharacterPlayable.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Traversal/GAFCharacterTraversalComponent.h"
#include "GAFGamePlayTag.h"
#include "GAFInputBindingHelpers.h"
#include "GAFInputSettings.h"
#include "GAFInputConfig.h"
#include "GAFPlayableLogChannels.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
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

void AGAFCharacterPlayable::SetupPlayerInputComponent(UInputComponent* Input)
{
	Super::SetupPlayerInputComponent(Input);

	UEnhancedInputComponent* EnhancedInput{ Cast<UEnhancedInputComponent>(Input) };
	if (!IsValid(EnhancedInput))
	{
		UE_LOG(LogGAFPlayable, Warning, TEXT("%s failed to SetupPlayerInputComponent: EnhancedInputComponent is invalid."), *GetNameSafe(this));
		return;
	}

	// 使用Helper绑定，按GamePlayTag来绑定输入，而不是直接绑定特定Action
	// 这样可以让InputAction与角色解耦
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_OnLookMouse, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Look_Mouse, ETriggerEvent::Canceled, this, &ThisClass::Input_OnLookMouse, false);

	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Look_Gamepad, ETriggerEvent::Triggered, this, &ThisClass::Input_OnLookGamepad, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Look_Gamepad, ETriggerEvent::Canceled, this, &ThisClass::Input_OnLookGamepad, false);

	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_OnMove, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Move, ETriggerEvent::Canceled, this, &ThisClass::Input_OnMove, false);

	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Move_WorldSpace, ETriggerEvent::Triggered, this, &ThisClass::Input_OnMoveWorldSpace, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Move_WorldSpace, ETriggerEvent::Canceled, this, &ThisClass::Input_OnMoveWorldSpace, false);

	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Walk, ETriggerEvent::Started, this, &ThisClass::Input_OnWalkPressed, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Walk, ETriggerEvent::Completed, this, &ThisClass::Input_OnWalkReleased, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Walk, ETriggerEvent::Canceled, this, &ThisClass::Input_OnWalkReleased, false);

	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Sprint, ETriggerEvent::Started, this, &ThisClass::Input_OnSprintPressed, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Sprint, ETriggerEvent::Completed, this, &ThisClass::Input_OnSprintReleased, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Sprint, ETriggerEvent::Canceled, this, &ThisClass::Input_OnSprintReleased, false);

	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Crouch, ETriggerEvent::Started, this, &ThisClass::Input_OnCrouchPressed, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Crouch, ETriggerEvent::Completed, this, &ThisClass::Input_OnCrouchReleased, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Crouch, ETriggerEvent::Canceled, this, &ThisClass::Input_OnCrouchReleased, false);

	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Aim, ETriggerEvent::Started, this, &ThisClass::Input_OnAimPressed, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Aim, ETriggerEvent::Completed, this, &ThisClass::Input_OnAimReleased, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Aim, ETriggerEvent::Canceled, this, &ThisClass::Input_OnAimReleased, false);

	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_ChangeRotationMode, ETriggerEvent::Started, this, &ThisClass::Input_OnChangeRotationModePressed, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_ChangeRotationMode, ETriggerEvent::Completed, this, &ThisClass::Input_OnChangeRotationModeReleased, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_ChangeRotationMode, ETriggerEvent::Canceled, this, &ThisClass::Input_OnChangeRotationModeReleased, false);

	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Jump, ETriggerEvent::Started, this, &ThisClass::Input_OnJumpStarted, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Jump, ETriggerEvent::Triggered, this, &ThisClass::Input_OnJumpTriggered, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Jump, ETriggerEvent::Completed, this, &ThisClass::Input_OnJumpReleased, false);
	GAFInput::BindNativeAction(EnhancedInput, InputConfig, GAFGamePlayTags::InputTag_Jump, ETriggerEvent::Canceled, this, &ThisClass::Input_OnJumpReleased, false);
}

void AGAFCharacterPlayable::HandleInputPressed(FGameplayTag InputTag)
{
	// 各类有效性检查
	if (!IsValid(InputConfig))
	{
		UE_LOG(LogGAFPlayable, Warning, TEXT("%s failed to handle input [%s]: InputConfig is invalid."), *GetNameSafe(this), *InputTag.ToString());

		return;
	}

	const FGAFInputAction* ActionConfig =
		InputConfig->FindNativeGAFInputActionForTag(InputTag, false);

	if (!ActionConfig)
	{
		UE_LOG(LogGAFPlayable, Warning, TEXT("%s failed to handle input [%s]: no native input action config found in [%s]."), *GetNameSafe(this), *InputTag.ToString(), *GetNameSafe(InputConfig));

		return;
	}

	if (!ActionConfig->InputStateTag.IsValid())
	{
		UE_LOG(LogGAFPlayable, Warning, TEXT("%s failed to handle input [%s]: action [%s] has no valid InputStateTag."), *GetNameSafe(this), *InputTag.ToString(), *GetNameSafe(ActionConfig->InputAction));

		return;
	}

	switch (ActionConfig->StateActivationMode)
	{
		// 按住触发就激活状态
		case EGAFInputStateActivationMode::Hold:
			SetInputStateTag(ActionConfig->InputStateTag, true);
			break;

		// 切换触发取反状态
		case EGAFInputStateActivationMode::Toggle:
			ToggleInputStateTag(ActionConfig->InputStateTag);
			break;

		default:
			break;
	}
}

void AGAFCharacterPlayable::HandleInputReleased(FGameplayTag InputTag)
{
	if (!IsValid(InputConfig))
	{
		UE_LOG(LogGAFPlayable, Warning, TEXT("%s failed to handle input release [%s]: InputConfig is invalid."), *GetNameSafe(this), *InputTag.ToString());

		return;
	}

	const FGAFInputAction* ActionConfig =
		InputConfig->FindNativeGAFInputActionForTag(InputTag, false);

	if (!ActionConfig)
	{
		UE_LOG(LogGAFPlayable, Warning, TEXT("%s failed to handle input release [%s]: no native input action config found in [%s]."), *GetNameSafe(this), *InputTag.ToString(), *GetNameSafe(InputConfig));

		return;
	}

	if (!ActionConfig->InputStateTag.IsValid())
	{
		UE_LOG(LogGAFPlayable, Warning, TEXT("%s failed to handle input release [%s]: action [%s] has no valid InputStateTag."), *GetNameSafe(this), *InputTag.ToString(), *GetNameSafe(ActionConfig->InputAction));

		return;
	}

	// 如果是按住触发那么就取消状态
	if (ActionConfig->StateActivationMode == EGAFInputStateActivationMode::Hold)
	{
		SetInputStateTag(ActionConfig->InputStateTag, false);
	}
}

void AGAFCharacterPlayable::Input_OnLookMouse(const FInputActionValue& ActionValue)
{
	const FVector2D Value{ ActionValue.Get<FVector2D>() };

	// 如果Config无效就回退到默认值
	// 同时默认Settings设为static，只初始化一次
	static const FGAFInputSettings DefaultLookSettings;
	const FGAFInputSettings& LookSettings{ IsValid(InputConfig) ? InputConfig->InputSettings : DefaultLookSettings };
	const float PitchSign{ LookSettings.bInvertPitchMouse ? -1.0f : 1.0f };
	const float YawSign{ LookSettings.bInvertYawMouse ? -1.0f : 1.0f };

	AddControllerPitchInput(Value.Y * PitchSign * LookSettings.MousePitchSensitivity);
	AddControllerYawInput(Value.X * YawSign * LookSettings.MouseYawSensitivity);
}

// 手柄额外需要DeltaSeconds处理
void AGAFCharacterPlayable::Input_OnLookGamepad(const FInputActionValue& ActionValue)
{
	const FVector2D Value{ ActionValue.Get<FVector2D>() };

	static const FGAFInputSettings DefaultLookSettings;
	const FGAFInputSettings& LookSettings{ IsValid(InputConfig) ? InputConfig->InputSettings : DefaultLookSettings };
	const float PitchSign{ LookSettings.bInvertPitchGamepad ? -1.0f : 1.0f };
	const float YawSign{ LookSettings.bInvertYawGamepad ? -1.0f : 1.0f };

	const float DeltaSeconds = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.0f;

	AddControllerPitchInput(Value.Y * PitchSign * LookSettings.GamepadPitchRate * DeltaSeconds);
	AddControllerYawInput(Value.X * YawSign * LookSettings.GamepadYawRate * DeltaSeconds);
}

void AGAFCharacterPlayable::Input_OnMove(const FInputActionValue& ActionValue)
{
	// TODO:
	// 这里不区分手柄轻推和推满，默认全部满输入
	FVector2D Value{ ActionValue.Get<FVector2D>() };
	if (Value.IsNearlyZero())
	{
		MoveInputLength = 0.0f;
		return;
	}

	// 设置遥感推动幅度
	MoveInputLength = Value.Length();
	Value.Normalize();

	const FRotator ControlRotation{ GetControlRotation() };
	const FRotator YawRotation{ 0.0, ControlRotation.Yaw, 0.0 };
	const auto ForwardDirection{ FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) };
	const auto RightDirection{ FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y) };

	AddMovementInput(RightDirection, Value.X);
	AddMovementInput(ForwardDirection, Value.Y);
}

void AGAFCharacterPlayable::Input_OnMoveWorldSpace(const FInputActionValue& ActionValue)
{
	FVector2D Value{ ActionValue.Get<FVector2D>() };
	if (Value.IsNearlyZero())
	{
		MoveWorldSpaceInputLength = 0.0f;
		return;
	}

	MoveWorldSpaceInputLength = Value.Length();
	Value.Normalize();

	// 直接加世界方向的输入
	AddMovementInput(FVector::RightVector, Value.X);
	AddMovementInput(FVector::ForwardVector, Value.Y);
}

void AGAFCharacterPlayable::Input_OnWalkPressed(const FInputActionValue& ActionValue)
{
	HandleInputPressed(GAFGamePlayTags::InputTag_Walk);
}

void AGAFCharacterPlayable::Input_OnWalkReleased(const FInputActionValue& ActionValue)
{
	HandleInputReleased(GAFGamePlayTags::InputTag_Walk);
}

void AGAFCharacterPlayable::Input_OnSprintPressed(const FInputActionValue& ActionValue)
{
	HandleInputPressed(GAFGamePlayTags::InputTag_Sprint);
}

void AGAFCharacterPlayable::Input_OnSprintReleased(const FInputActionValue& ActionValue)
{
	HandleInputReleased(GAFGamePlayTags::InputTag_Sprint);
}

void AGAFCharacterPlayable::Input_OnCrouchPressed(const FInputActionValue& ActionValue)
{
	HandleInputPressed(GAFGamePlayTags::InputTag_Crouch);
	RefreshCrouchFromInputState();
}

void AGAFCharacterPlayable::Input_OnCrouchReleased(const FInputActionValue& ActionValue)
{
	HandleInputReleased(GAFGamePlayTags::InputTag_Crouch);
	RefreshCrouchFromInputState();
}

void AGAFCharacterPlayable::Input_OnAimPressed(const FInputActionValue& ActionValue)
{
	HandleInputPressed(GAFGamePlayTags::InputTag_Aim);
}

void AGAFCharacterPlayable::Input_OnAimReleased(const FInputActionValue& ActionValue)
{
	HandleInputReleased(GAFGamePlayTags::InputTag_Aim);
}

void AGAFCharacterPlayable::Input_OnChangeRotationModePressed(const FInputActionValue& ActionValue)
{
	HandleInputPressed(GAFGamePlayTags::InputTag_ChangeRotationMode);
}

void AGAFCharacterPlayable::Input_OnChangeRotationModeReleased(const FInputActionValue& ActionValue)
{
	HandleInputReleased(GAFGamePlayTags::InputTag_ChangeRotationMode);
}

void AGAFCharacterPlayable::Input_OnJumpStarted(const FInputActionValue& ActionValue)
{
	(void)ActionValue;

	FGAFTraversalCheckResult TraversalCheckResult = FGAFTraversalCheckResult();

	// 第一次按下时先尝试 Traversal，Traversal 失败才执行普通 Jump
	if (!CharacterTraversalComponent->TryTraversalAction(
			GetTraversalCheckInputs(),
			GetDefaultCharacterSettings().TraversalSettings,
			GetDefaultCharacterSettings().MotionMatchingSettings,
			EDrawDebugTrace::ForDuration,
			TraversalCheckResult))
	{
		CharacterTraversalComponent->DebugPrintTraversalFailureReason(
			TraversalCheckResult,
			GetDefaultCharacterSettings().TraversalSettings);
		Jump();
	}
}

void AGAFCharacterPlayable::Input_OnJumpTriggered(const FInputActionValue& ActionValue)
{
	(void)ActionValue;

	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!IsValid(Movement))
	{
		UE_LOG(LogGAFPlayable, Warning, TEXT("%s failed to handle jump triggered: CMC is invalid."), *GetNameSafe(this));
		return;
	}

	// 按住时在空中持续检测 Traversal；失败不会触发 Jump
	if (Movement->IsFalling())
	{
		FGAFTraversalCheckResult TraversalCheckResult = FGAFTraversalCheckResult();
		const bool bTraversalSucceeded = CharacterTraversalComponent->TryTraversalAction(
			GetTraversalCheckInputs(),
			GetDefaultCharacterSettings().TraversalSettings,
			GetDefaultCharacterSettings().MotionMatchingSettings,
			EDrawDebugTrace::ForOneFrame,
			TraversalCheckResult);

		if (!bTraversalSucceeded)
		{
			CharacterTraversalComponent->DebugPrintTraversalFailureReason(
				TraversalCheckResult,
				GetDefaultCharacterSettings().TraversalSettings);
		}
	}
}

void AGAFCharacterPlayable::Input_OnJumpReleased(const FInputActionValue& ActionValue)
{
	(void)ActionValue;

	StopJumping();
}
