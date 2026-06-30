#pragma once

#include "GAFInputSettings.generated.h"

USTRUCT(BlueprintType)
struct GAFINPUT_API FGAFInputSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LookSettings|Mouse")
	float MousePitchSensitivity{ 1.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LookSettings|Mouse")
	float MouseYawSensitivity{ 1.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LookSettings|Gamepad")
	float GamepadPitchRate{ 90.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LookSettings|Gamepad")
	float GamepadYawRate{ 240.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LookSettings|Invert")
	bool bInvertPitch{ false };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LookSettings|Invert")
	bool bInvertYaw{ false };
};