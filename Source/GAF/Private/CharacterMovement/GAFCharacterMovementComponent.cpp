#include "CharacterMovement/GAFCharacterMovementComponent.h"

#include "Character/GAFCharacterCore.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveVector.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFCharacterMovementComponent)

UGAFCharacterMovementComponent::UGAFCharacterMovementComponent()
{
}

FVector UGAFCharacterMovementComponent::ConsumeInputVector()
{
	FVector InputVector{ Super::ConsumeInputVector() };

	// 其他系统接管角色时丢弃输入值
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

// 这个函数在 PhysicsRotation PhysWalking 前被调用
// 所以效果应该与 GASP 里 Add Tick Prerequisite 作用一致，都是先更新运动参数
// 获取参数后缓存下来，在PhysicsRotation/Walking里再分别应用移动/旋转的参数
void UGAFCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	CachedLocomotionData = FGAFLocomotionData{};

	// BlueprintNativeEvent 接口函数必须通过 Execute_ 静态函数调用，不能直接调用接口成员函数
	const bool bHasData = IGAFLocomotionDataProvider::Execute_GetLocomotionData(CharacterOwner, CachedLocomotionData);

	// 数据无效
	// TODO:考虑使用上一帧数据而不是直接用默认值
	if (!bHasData)
	{
		CachedLocomotionData = FGAFLocomotionData{};
	}

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UGAFCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	bUseControllerDesiredRotation = CachedLocomotionData.bUseControllerDesiredRotation;
	bOrientRotationToMovement = CachedLocomotionData.bOrientRotationToMovement;
	RotationRate = CachedLocomotionData.RotationRate;

	Super::PhysicsRotation(DeltaTime);
}

void UGAFCharacterMovementComponent::PhysWalking(float DeltaTime, int32 IterationsCount)
{
	MaxAcceleration = CachedLocomotionData.MaxAcceleration;
	BrakingDecelerationWalking = CachedLocomotionData.BrakingDecelerationWalking;
	GroundFriction = CachedLocomotionData.GroundFriction;
	MaxWalkSpeed = CachedLocomotionData.MaxWalkSpeed;
	MaxWalkSpeedCrouched = CachedLocomotionData.MaxWalkSpeedCrouched;

	Super::PhysWalking(DeltaTime, IterationsCount);
}
