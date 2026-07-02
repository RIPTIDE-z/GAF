#pragma once

#include "GameplayTagContainer.h"
#include "MotionWarpingComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GAFAnimationTypes.generated.h"

USTRUCT(BlueprintType)
struct GAF_API FGAFAnimationFrameData
{
	GENERATED_BODY()

	// 输入状态
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayTagContainer InputStateTags;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly)
	FGameplayTag MovementMode;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly)
	FGameplayTag Stance;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly)
	FGameplayTag RotationMode;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly)
	FGameplayTag Gait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Velocity{ ForceInit };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector InputAcceleration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FTransform ActorTransform{ FTransform::Identity };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentMaxAcceleration{ 0.0f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentMaxDeceleration{ 0.0f };

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

// TODO: 摄像机
USTRUCT(BlueprintType)
struct GAF_API FGAFCameraFrameData
{
	GENERATED_BODY()
};

// TODO: 翻越系统
USTRUCT(BlueprintType)
struct GAF_API FGAFTraversalFrameData
{
	GENERATED_BODY()

	// 角色胶囊体
	UPROPERTY(BlueprintReadOnly)
	UCapsuleComponent* Capsule;

	// 角色网格体组件
	UPROPERTY(BlueprintReadOnly)
	USkeletalMeshComponent* Mesh;

	// MotionWarping组件
	UPROPERTY(BlueprintReadOnly)
	UMotionWarpingComponent* MotionWarping;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag MovementMode;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Gait;

	UPROPERTY(BlueprintReadOnly)
	float Speed{ 0.0f };
};

// 给 CMC 使用的运动数据，包含移动和旋转
USTRUCT(BlueprintType)
struct FGAFLocomotionData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float MaxAcceleration{ 0.0f };

	UPROPERTY(BlueprintReadOnly)
	float BrakingDecelerationWalking{ 0.0f };

	UPROPERTY(BlueprintReadOnly)
	float GroundFriction{ 0.0f };

	UPROPERTY(BlueprintReadOnly)
	float MaxWalkSpeed{ 0.0f };

	UPROPERTY(BlueprintReadOnly)
	float MaxWalkSpeedCrouched{ 0.0f };

	UPROPERTY(BlueprintReadOnly)
	bool bUseControllerDesiredRotation{ true };

	UPROPERTY(BlueprintReadOnly)
	bool bOrientRotationToMovement{ false };

	UPROPERTY(BlueprintReadOnly)
	FRotator RotationRate{ 0.0f, -1.0f, 0.0f };
};
