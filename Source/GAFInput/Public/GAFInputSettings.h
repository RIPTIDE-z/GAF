#pragma once

#include "GAFInputSettings.generated.h"

USTRUCT(BlueprintType)
struct GAFINPUT_API FGAFInputSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LookSettings|Mouse",
		meta = (ClampMin = "1.0", ClampMax = "10.0", UIMin = "1.0", UIMax = "10.0"))
	float MousePitchSensitivity{ 1.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LookSettings|Mouse",
		meta = (ClampMin = "1.0", ClampMax = "10.0", UIMin = "1.0", UIMax = "10.0"))
	float MouseYawSensitivity{ 1.0f };
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LookSettings|Mouse")
	bool bInvertPitchMouse{ false };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LookSettings|Mouse")
	bool bInvertYawMouse{ false };

	// 代表每秒旋转度数
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LookSettings|Gamepad",
		meta = (ClampMin = "1.0", ClampMax = "720.0", UIMin = "1.0", UIMax = "720.0", ForceUnits = "deg/s"))
	float GamepadPitchRate{ 90.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LookSettings|Gamepad",
		meta = (ClampMin = "1.0", ClampMax = "720.0", UIMin = "1.0", UIMax = "720.0", ForceUnits = "deg/s"))
	float GamepadYawRate{ 90.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LookSettings|Gamepad")
	bool bInvertPitchGamepad{ false };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LookSettings|Gamepad")
	bool bInvertYawGamepad{ false };
};