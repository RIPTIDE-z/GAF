#pragma once

#include "GAFViewState.generated.h"

// Stores character view information
USTRUCT(BlueprintType)
struct GAF_API FGAFViewState
{
	GENERATED_BODY()

	// Network
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAF")
	// FGAFViewNetworkSmoothingState NetworkSmoothing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAF")
	FRotator Rotation{ ForceInit };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAF", Meta = (ClampMin = 0, ForceUnits = "deg/s"))
	float YawSpeed{ 0.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAF", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float PreviousYawAngle{ 0.0f };
};
