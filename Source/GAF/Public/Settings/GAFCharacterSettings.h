#pragma once

#include "Settings/GAFFootPlacementSettings.h"
#include "Settings/GAFMovementSettings.h"
#include "Settings/GAFMotionMatchingSettings.h"
#include "Settings/GAFTraversalSettings.h"
#include "Engine/DataAsset.h"

#include "GAFCharacterSettings.generated.h"

UCLASS(BlueprintType)
class GAF_API UGAFCharacterSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	UGAFCharacterSettings(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAF")
	FGAFMovementSettings MovementSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAF")
	FGAFTraversalSettings TraversalSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAF")
	FGAFMotionMatchingSettings MotionMatchingSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAF")
	FGAFFootPlacementSettings FootPlacementSettings;
};
