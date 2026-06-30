#pragma once

#include "GAFMovementSettings.generated.h"

USTRUCT(BlueprintType)
struct GAF_API FGAFMovementSettings
{
	GENERATED_BODY()
	

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Speed")
	FVector WalkSpeeds;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Speed")
	FVector RunSpeeds;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Speed")
	FVector SprintSpeeds;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Speed")
	FVector CrouchSpeeds;
};