#pragma once
#include "GameplayTagContainer.h"

#include "GAFAnimationTypes.generated.h"
#include "GAFCharacterMovementComponent.h"

USTRUCT(BlueprintType)
struct GAF_API FGAFAnimationFrameData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly)
	FGameplayTag MovementMode;
	
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly)
	FGameplayTag Stance;
	
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly)
	FGameplayTag RotationMode;
	
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly)
	FGameplayTag Gait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Velocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector InputAcceleration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FTransform ActorTransform = FTransform::Identity;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentMaxAcceleration;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentMaxDeceleration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FRotator OrientationIntent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FRotator AimingRotation;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool JustLanded;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector LandVelocity;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector GroundNormal;
};
