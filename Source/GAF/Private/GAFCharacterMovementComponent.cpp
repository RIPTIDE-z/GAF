#include "GAFCharacterMovementComponent.h"

#include "Character/GAFCharacterCore.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveVector.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFCharacterMovementComponent)

UGAFCharacterMovementComponent::UGAFCharacterMovementComponent()
{
	// Default values for standing walking movement

	MinAnalogWalkSpeed = 25.0f;
	MaxWalkSpeed = 375.0f;
	MaxWalkSpeedCrouched = 150.0f;
	MaxAccelerationWalking = 2000.0f;
	BrakingDecelerationWalking = 1500.0f;
	GroundFriction = 4.0f;

	// Use GroundFriction and FallingLateralFriction for both acceleration and deceleration
	bUseSeparateBrakingFriction = false;

	// Keep braking friction disabled by default except for short landing windows
	BrakingFrictionFactor = 0.0f;

	// Prevent the movement component from driving actor rotation

	RotationRate = FRotator::ZeroRotator;
	bUseControllerDesiredRotation = false;
	bOrientRotationToMovement = false;
}

FVector UGAFCharacterMovementComponent::ConsumeInputVector()
{
	auto InputVector{ Super::ConsumeInputVector() };

	// Drop input while movement is blocked by another character state
	if (bInputBlocked)
	{
		InputVector = FVector::ZeroVector;
		return InputVector;
	}

	// TODO:MovementBase

	return InputVector;
}

void UGAFCharacterMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
	// TODO:MovementBase
	Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
}

void UGAFCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	Super::PhysicsRotation(DeltaTime);
}

void UGAFCharacterMovementComponent::PhysWalking(float DeltaTime, int32 IterationsCount)
{
	Super::PhysWalking(DeltaTime, IterationsCount);
}
