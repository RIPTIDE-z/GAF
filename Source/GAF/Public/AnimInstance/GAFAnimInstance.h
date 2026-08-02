#pragma once

#include "Animation/AnimInstance.h"
#include "Animation/GAFAnimationTypes.h"
#include "Animation/TrajectoryTypes.h"
#include "AnimInstance/GAFAnimInstanceDataProvider.h"
#include "PoseSearch/PoseSearchHistory.h"
#include "PoseSearch/PoseSearchTrajectoryLibrary.h"

#include "GAFAnimInstance.generated.h"

class UGAFAnimInstanceSettings;
class UPoseSearchDatabase;

UCLASS()
class GAF_API UGAFAnimationInstance :
	public UAnimInstance,
	public IGAFAnimInstanceDataProvider
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;

	virtual void NativeUpdateAnimation(float DeltaTime) override;

	virtual void NativeThreadSafeUpdateAnimation(float DeltaTime) override;

	// IGAFAnimInstanceDataProvider 的 C++ 默认实现
	virtual void SetTraversalInteractionTransform_Implementation(const FTransform& InInteractionTransform) override;

	const FTransform& GetTraversalInteractionTransform() const { return TraversalInteractionTransform; }

	virtual bool GetTraversalPoseHistoryReference_Implementation(FName PoseHistoryTag, FPoseHistoryReference& OutPoseHistory) override;

protected:
	void UpdateTrajectory(float DeltaTime);
	void UpdateEssentialValues(float DeltaTime);
	void UpdateStates();

	UFUNCTION(BlueprintPure, Category = "GAF|Animation|States", Meta = (BlueprintThreadSafe))
	bool IsMoving() const;

	UFUNCTION(BlueprintPure, Category = "GAF|Animation|Trajectory", Meta = (BlueprintThreadSafe))
	FQuat GetDesiredFacing() const;

	// Essential values gathered on the game thread for animation graph consumption
	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Essential Values")
	FGAFAnimationFrameData AnimationFrameData;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Essential Values")
	FTransform CharacterTransform{ FTransform::Identity };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Essential Values")
	FTransform CharacterTransformLastFrame{ FTransform::Identity };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Essential Values")
	FTransform RootTransform{ FTransform::Identity };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Essential Values")
	bool bHasAcceleration{ false };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Essential Values")
	FVector Acceleration{ ForceInit };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Essential Values")
	FVector AccelerationLastFrame{ ForceInit };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Essential Values")
	float AccelerationAmount{ 0.0f };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Essential Values")
	bool bHasVelocity{ false };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Essential Values")
	FVector Velocity{ ForceInit };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Essential Values")
	FVector VelocityLastFrame{ ForceInit };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Essential Values")
	FVector RelativeAcceleration{ ForceInit };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Essential Values")
	FVector VelocityAcceleration{ ForceInit };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Essential Values")
	FVector LastNonZeroVelocity{ ForceInit };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Essential Values")
	float Speed2D{ 0.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAF|Animation|Essential Values", Meta = (ClampMin = "0.0"))
	float HeavyLandSpeedThreshold{ 0.0f };

	// Traversal Motion Matching 使用的交互目标，由 CharacterTraversalComponent 在检测成功后写入
	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Traversal")
	FTransform TraversalInteractionTransform{ FTransform::Identity };

	// 可用于开关 OffSetRootBone
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAF|Animation|Essential Values")
	bool bOffsetRootBoneEnabled{ false };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAF|Animation|Essential Values")
	FName OffsetRootBoneNodeTag{ TEXT("OffsetRootBone") };

	// Gameplay tags 代表的运动模式、状态
	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|States")
	FGameplayTag MovementMode;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|States")
	FGameplayTag MovementModeLastFrame;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|States")
	FGameplayTag RotationMode;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|States")
	FGameplayTag RotationModeLastFrame;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|States")
	FGameplayTag MovementState;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|States")
	FGameplayTag MovementStateLastFrame;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|States")
	FGameplayTag Gait;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|States")
	FGameplayTag GaitLastFrame;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|States")
	FGameplayTag Stance;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|States")
	FGameplayTag StanceLastFrame;

	// Trajectory 数据
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAF|Animation|Trajectory")
	FPoseSearchTrajectoryData IdleTrajectoryGenerationData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAF|Animation|Trajectory")
	FPoseSearchTrajectoryData MovingTrajectoryGenerationData;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Trajectory")
	FTransformTrajectory Trajectory;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Trajectory")
	FPoseSearchTrajectory_WorldCollisionResults TrajectoryCollision;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Trajectory")
	float PreviousDesiredControllerYaw{ 0.0f };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Trajectory")
	FVector TrajectoryPastVelocity{ ForceInit };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Trajectory")
	FVector TrajectoryCurrentVelocity{ ForceInit };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Trajectory")
	FVector TrajectoryFutureVelocity{ ForceInit };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAF|Animation|Root Offset", Meta = (ClampMin = "0.0"))
	float OffsetRootTranslationRadius{ 0.0f };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Foot Placement")
	int32 FootPlacementMode{ 0 };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAF|Animation|Foot Placement")
	bool bFootPlacementEnabled{ true };

	// Runtime Motion Matching selection state
	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Motion Matching")
	int32 MotionMatchingDatabaseLOD{ 0 };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Motion Matching")
	TObjectPtr<UObject> CurrentSelectedAnimation{ nullptr };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Motion Matching")
	TObjectPtr<const UPoseSearchDatabase> CurrentSelectedDatabase{ nullptr };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Motion Matching")
	TArray<TObjectPtr<const UPoseSearchDatabase>> ValidDatabases;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Motion Matching")
	float MotionMatchingSearchCost{ 0.0f };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Animation|Motion Matching")
	TArray<FName> CurrentDatabaseTags;

private:
	bool bAnimationFrameDataValid{ false };
	bool bReportedMissingDataProvider{ false };
	bool bReportedFrameDataFailure{ false };
	bool bReportedTrajectoryFailure{ false };
};
