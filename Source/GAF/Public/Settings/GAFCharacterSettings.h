#pragma once

#include "Component/GAFCharacterMovementComponent.h"
#include "Settings/GAFMovementSettings.h"
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
};