#pragma once

#include "GAFALSViewState.generated.h"

// 存储角色的视角信息
USTRUCT(BlueprintType)
struct GAFALS_API FGAFALSViewState
{
	GENERATED_BODY()

	// Network
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAFALS")
	// FGAFALSViewNetworkSmoothingState NetworkSmoothing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAFALS")
	FRotator Rotation{ ForceInit };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAFALS", Meta = (ClampMin = 0, ForceUnits = "deg/s"))
	float YawSpeed{ 0.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAFALS", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float PreviousYawAngle{ 0.0f };
};