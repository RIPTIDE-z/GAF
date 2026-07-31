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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Animation")
	FGameplayTagContainer InputStateTags;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Animation")
	FGameplayTag MovementMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Animation")
	FGameplayTag Stance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Animation")
	FGameplayTag RotationMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Animation")
	FGameplayTag Gait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Animation")
	FTransform ActorTransform{ FTransform::Identity };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Animation")
	FVector Velocity{ ForceInit };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Animation")
	FVector InputAcceleration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Animation")
	float CurrentMaxAcceleration{ 0.0f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Animation")
	float CurrentMaxDeceleration{ 0.0f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Animation")
	FRotator OrientationIntent{ 0.0f, 0.0f, 0.0f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Animation")
	FRotator AimingRotation{ 0.0f, 0.0f, 0.0f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Animation")
	bool JustLanded{ false };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Animation")
	FVector LandVelocity{ ForceInit };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Animation")
	FVector GroundNormal{ ForceInit };
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
	UPROPERTY(BlueprintReadOnly, Category = "GAF|Traversal")
	UCapsuleComponent* Capsule;

	// 角色网格体组件
	UPROPERTY(BlueprintReadOnly, Category = "GAF|Traversal")
	USkeletalMeshComponent* Mesh;

	// MotionWarping组件
	UPROPERTY(BlueprintReadOnly, Category = "GAF|Traversal")
	UMotionWarpingComponent* MotionWarping;

	UPROPERTY(BlueprintReadOnly, Category = "GAF|Traversal")
	FGameplayTag MovementMode;

	UPROPERTY(BlueprintReadOnly, Category = "GAF|Traversal")
	FGameplayTag Gait;

	UPROPERTY(BlueprintReadOnly, Category = "GAF|Traversal")
	float Speed{ 0.0f };
};

// 给 CMC 使用的运动数据，包含移动和旋转
USTRUCT(BlueprintType)
struct FGAFLocomotionData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "GAF|Movement")
	float MaxAcceleration{ 0.0f };

	UPROPERTY(BlueprintReadOnly, Category = "GAF|Movement")
	float BrakingDecelerationWalking{ 0.0f };

	UPROPERTY(BlueprintReadOnly, Category = "GAF|Movement")
	float GroundFriction{ 0.0f };

	UPROPERTY(BlueprintReadOnly, Category = "GAF|Movement")
	float MaxWalkSpeed{ 0.0f };

	UPROPERTY(BlueprintReadOnly, Category = "GAF|Movement")
	float MaxWalkSpeedCrouched{ 0.0f };

	UPROPERTY(BlueprintReadOnly, Category = "GAF|Movement")
	bool bUseControllerDesiredRotation{ true };

	UPROPERTY(BlueprintReadOnly, Category = "GAF|Movement")
	bool bOrientRotationToMovement{ false };

	UPROPERTY(BlueprintReadOnly, Category = "GAF|Movement")
	FRotator RotationRate{ 0.0f, -1.0f, 0.0f };
};
