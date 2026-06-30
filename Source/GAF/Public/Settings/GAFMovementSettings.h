#pragma once

#include "GAFMovementSettings.generated.h"

USTRUCT(BlueprintType)
struct GAF_API FGAFMovementSettings
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Speed")
	TObjectPtr<UCurveFloat> StrafeSpeedMapCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Speed")
	FVector WalkSpeeds;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Speed")
	FVector RunSpeeds;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Speed")
	FVector SprintSpeeds;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Speed")
	FVector CrouchSpeeds;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float MaxAcceleration{500.f};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Braking")
	bool UseSeperateBrakingFriction{true};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Braking")
	float BrakingDecelerationWalking{440.f};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Braking")
	float BrakingFriction{0.f};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Braking")
	float BrakingFrictionFactor{1.f};
};