#pragma once

#include "BoneControllers/AnimNode_FootPlacement.h"

#include "GAFFootPlacementSettings.generated.h"

USTRUCT(BlueprintType)
struct GAF_API FGAFFootPlacementSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Foot Placement")
	FFootPlacementPlantSettings DefaultPlantSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Foot Placement")
	FFootPlacementPlantSettings StopPlantSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Foot Placement")
	FFootPlacementInterpolationSettings DefaultInterpolationSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Foot Placement")
	FFootPlacementInterpolationSettings StopInterpolationSettings;
};
