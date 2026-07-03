#pragma once

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (ShowOnlyInnerProperties))
	FGAFMovementSettings MovementSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (ShowOnlyInnerProperties))
	FGAFTraversalSettings TraversalSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (ShowOnlyInnerProperties))
	FGAFMotionMatchingSettings MotionMatchingSettings;
};
