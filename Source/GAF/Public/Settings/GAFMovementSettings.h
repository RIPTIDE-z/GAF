#pragma once

#include "GAFMovementSettings.generated.h"

USTRUCT(BlueprintType)
struct GAF_API FGAFMovementSettings
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Speed")
	TObjectPtr<UCurveFloat> StrafeSpeedMapCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Speed")
	FVector WalkSpeeds{200.0, 180.0, 150.0};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Speed")
	FVector RunSpeeds{500.0, 350.0, 300.0};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Speed")
	FVector SprintSpeeds{700.0, 700.0, 700.0};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Speed")
	FVector CrouchSpeeds{225.0, 200.0, 180.0};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float MaxAcceleration{500.f};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Braking")
	bool UseSeparateBrakingFriction{true};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Braking")
	float BrakingDecelerationWalking{440.f};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Braking")
	float BrakingFriction{0.f};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Braking")
	float BrakingFrictionFactor{1.f};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Rotation")
	bool UseControllerDesiredRotation{false};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Rotation")
	bool OrientRotationToMovement{false};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Rotation")
	bool DefaultStrafe{true};
};