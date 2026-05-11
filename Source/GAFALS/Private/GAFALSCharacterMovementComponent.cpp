#include "GAFALSCharacterMovementComponent.h"

#include "Character/GAFALSCharacterCore.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveVector.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "Utility/GAFALSVector.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFALSCharacterMovementComponent)

UGAFALSCharacterMovementComponent::UGAFALSCharacterMovementComponent()
{
	// Default values for standing walking movement.

	MinAnalogWalkSpeed = 25.0f;
	MaxWalkSpeed = 375.0f;
	MaxWalkSpeedCrouched = 150.0f;
	MaxAccelerationWalking = 2000.0f;
	BrakingDecelerationWalking = 1500.0f;
	GroundFriction = 4.0f;

	// Makes GroundFriction and FallingLateralFriction used for both acceleration and deceleration.
	bUseSeparateBrakingFriction = false;

	// Makes friction does not affect deceleration by default. Greater than zero only for a short period of time after landing.
	BrakingFrictionFactor = 0.0f;

	// These values prohibit the character movement component from affecting the actor's rotation.

	RotationRate = FRotator::ZeroRotator;
	bUseControllerDesiredRotation = false;
	bOrientRotationToMovement = false;
}

FVector UGAFALSCharacterMovementComponent::ConsumeInputVector()
{
	auto InputVector{ Super::ConsumeInputVector() };

	// 先判断移动功能是否被禁用，比如布娃娃状态
	if (bInputBlocked)
	{
		InputVector = FVector::ZeroVector;
		return InputVector;
	}

	// TODO:MovementBase

	return InputVector;
}

void UGAFALSCharacterMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
	// TODO:MovementBase
	Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
}

void UGAFALSCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	Super::PhysicsRotation(DeltaTime);
}

void UGAFALSCharacterMovementComponent::PhysWalking(float DeltaTime, int32 IterationsCount)
{
	Super::PhysWalking(DeltaTime, IterationsCount);
}