#pragma once

#include "GAFInputSettings.generated.h"

USTRUCT(BlueprintType)
struct GAFINPUT_API FGAFInputSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Look|Mouse")
	float MousePitchSensitivity{ 1.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Look|Mouse")
	float MouseYawSensitivity{ 1.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Look|Gamepad")
	float GamepadPitchRate{ 90.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Look|Gamepad")
	float GamepadYawRate{ 240.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Look|Invert")
	bool bInvertPitch{ false };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Look|Invert")
	bool bInvertYaw{ false };
};
