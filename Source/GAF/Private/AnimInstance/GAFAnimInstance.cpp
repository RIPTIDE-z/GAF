#include "AnimInstance/GAFAnimInstance.h"

#include "GAFGamePlayTag.h"
#include "GAFLogChannels.h"
#include "Animation/AnimClassInterface.h"
#include "Animation/AnimSubsystem_Tag.h"
#include "Animation/GAFCharacterDataProvider.h"
#include "AnimationWarpingLibrary.h"
#include "BoneControllers/AnimNode_OffsetRootBone.h"
#include "Engine/EngineTypes.h"
#include "PoseSearch/AnimNode_PoseSearchHistoryCollector.h"
#include "PoseSearch/PoseSearchLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFAnimInstance)

void UGAFAnimationInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
}

void UGAFAnimationInstance::NativeUpdateAnimation(const float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	bAnimationFrameDataValid = false;

	// 检查 Owner 是否有效以及是否实现了数据获取接口
	AActor* OwningActor = GetOwningActor();
	if (!IsValid(OwningActor) || !OwningActor->GetClass()->ImplementsInterface(UGAFCharacterDataProvider::StaticClass()))
	{
		if (!bReportedMissingDataProvider)
		{
			UE_LOG(LogGAFAnimation, Error, TEXT("%s cannot update animation frame data: the owning actor is invalid or does not implement GAFCharacterDataProvider."), *GetNameSafe(this));
			bReportedMissingDataProvider = true;
		}
		return;
	}
	bReportedMissingDataProvider = false;

	// 调用接口获取 AnimationFrameData
	bAnimationFrameDataValid = IGAFCharacterDataProvider::Execute_GetAnimationFrameData(OwningActor, AnimationFrameData);
	if (!bAnimationFrameDataValid)
	{
		if (!bReportedFrameDataFailure)
		{
			UE_LOG(LogGAFAnimation, Error, TEXT("%s failed to get animation frame data from %s."), *GetNameSafe(this), *GetNameSafe(OwningActor));
			bReportedFrameDataFailure = true;
		}
		return;
	}
	bReportedFrameDataFailure = false;
}

void UGAFAnimationInstance::NativeThreadSafeUpdateAnimation(const float DeltaTime)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaTime);

	if (!bAnimationFrameDataValid)
	{
		return;
	}

	UpdateTrajectory(DeltaTime);
	UpdateEssentialValues(DeltaTime);
	UpdateStates();
}

// 更新用于 MotionMatching 的轨迹数据
void UGAFAnimationInstance::UpdateTrajectory(const float DeltaTime)
{
	if (!FMath::IsFinite(DeltaTime) || DeltaTime < 0.0f)
	{
		UE_LOG(LogGAFAnimation, Error, TEXT("UpdateTrajectory received an invalid DeltaTime: %f."), DeltaTime);
		return;
	}

	// 区分 Idle 和 Moving 的数据
	const FPoseSearchTrajectoryData& TrajectoryGenerationData =
		Speed2D > 0.0f ? MovingTrajectoryGenerationData : IdleTrajectoryGenerationData;

	// 生成 Trajectory 数据并写入
	// TODO:参数配置放入数据资产
	FTransformTrajectory GeneratedTrajectory;
	UPoseSearchTrajectoryLibrary::PoseSearchGenerateTransformTrajectory(
		this,
		TrajectoryGenerationData,
		DeltaTime,
		Trajectory,
		PreviousDesiredControllerYaw,
		GeneratedTrajectory,
		-1.0f,
		30,
		0.1f,
		15);

	// 进行一次碰撞处理
	FTransformTrajectory CollisionAdjustedTrajectory;
	const TArray<AActor*> ActorsToIgnore;
	UPoseSearchTrajectoryLibrary::HandleTransformTrajectoryWorldCollisions(
		this,
		this,
		GeneratedTrajectory,
		true,
		0.01f,
		CollisionAdjustedTrajectory,
		TrajectoryCollision,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		true,
		150.0f,
		FLinearColor::Red,
		FLinearColor::Green,
		5.0f);

	Trajectory = MoveTemp(CollisionAdjustedTrajectory);
	if (Trajectory.Samples.IsEmpty())
	{
		if (!bReportedTrajectoryFailure)
		{
			UE_LOG(LogGAFAnimation, Error, TEXT("UpdateTrajectory produced no trajectory samples."));
			bReportedTrajectoryFailure = true;
		}
		TrajectoryPastVelocity = FVector::ZeroVector;
		TrajectoryCurrentVelocity = FVector::ZeroVector;
		TrajectoryFutureVelocity = FVector::ZeroVector;
		return;
	}
	bReportedTrajectoryFailure = false;

	// 从 Trajectory 数据中获取速度信息
	UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(Trajectory, -0.3f, -0.2f, TrajectoryPastVelocity, false);
	UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(Trajectory, 0.0f, 0.2f, TrajectoryCurrentVelocity, false);
	UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(Trajectory, 0.4f, 0.5f, TrajectoryFutureVelocity, false);
}

// 更新重要数据
void UGAFAnimationInstance::UpdateEssentialValues(const float DeltaTime)
{
	CharacterTransformLastFrame = CharacterTransform;
	CharacterTransform = AnimationFrameData.ActorTransform;

	RootTransform = CharacterTransform;
	if (bOffsetRootBoneEnabled)
	{
		FAnimNode_OffsetRootBone* OffsetRootBoneNode = nullptr;
		if (IAnimClassInterface* AnimBlueprintClass = IAnimClassInterface::GetFromClass(GetClass()))
		{
			if (const FAnimSubsystem_Tag* TagSubsystem = AnimBlueprintClass->FindSubsystem<FAnimSubsystem_Tag>())
			{
				// 与 GASP 逻辑保持一致，使用 Tag 来找到 OffSetRootBone 节点
				OffsetRootBoneNode = TagSubsystem->FindNodeByTag<FAnimNode_OffsetRootBone>(OffsetRootBoneNodeTag, this);
			}
		}

		if (OffsetRootBoneNode != nullptr)
		{
			const FAnimNodeReference OffsetRootBoneReference{ this, *OffsetRootBoneNode };
			const FTransform OffsetRootTransform = UAnimationWarpingLibrary::GetOffsetRootTransform(OffsetRootBoneReference);
			FRotator OffsetRootRotation = OffsetRootTransform.Rotator();
			OffsetRootRotation.Yaw += 90.0f;
			RootTransform = FTransform{ OffsetRootRotation, OffsetRootTransform.GetLocation(), FVector::OneVector };
		}
		else
		{
			UE_LOG(LogGAFAnimation, Warning, TEXT("Offset Root Bone node tagged [%s] was not found."), *OffsetRootBoneNodeTag.ToString());
		}
	}

	AccelerationLastFrame = Acceleration;
	Acceleration = AnimationFrameData.InputAcceleration;
	AccelerationAmount = AnimationFrameData.CurrentMaxAcceleration > UE_SMALL_NUMBER
		? Acceleration.Size() / AnimationFrameData.CurrentMaxAcceleration
		: 0.0f;
	bHasAcceleration = AccelerationAmount > 0.0f;

	VelocityLastFrame = Velocity;
	Velocity = AnimationFrameData.Velocity;
	Speed2D = Velocity.Size2D();
	bHasVelocity = Speed2D > 5.0f;

	const float SafeDeltaTime = FMath::Max(DeltaTime, 0.001f);
	VelocityAcceleration = (Velocity - VelocityLastFrame) / SafeDeltaTime;
	RelativeAcceleration = RootTransform.GetRotation().UnrotateVector(VelocityAcceleration);

	if (bHasVelocity)
	{
		LastNonZeroVelocity = Velocity;
	}
}

// 更新运动状态
void UGAFAnimationInstance::UpdateStates()
{
	MovementModeLastFrame = MovementMode;
	MovementMode = AnimationFrameData.MovementMode;

	RotationModeLastFrame = RotationMode;
	RotationMode = AnimationFrameData.RotationMode;

	MovementStateLastFrame = MovementState;
	MovementState = IsMoving()
		? GAFGamePlayTags::MovementState_Moving
		: GAFGamePlayTags::MovementState_Idle;

	GaitLastFrame = Gait;
	Gait = AnimationFrameData.Gait;

	StanceLastFrame = Stance;
	Stance = AnimationFrameData.Stance;
}

bool UGAFAnimationInstance::IsMoving() const
{
	// 速度以及加速度均不为 0 则表示在运动
	return !Velocity.Equals(FVector::ZeroVector, 0.1f) && !Acceleration.Equals(FVector::ZeroVector, 0.0f);
}

// 取 Trajectory 里未来 0.5s 处的 facing
FQuat UGAFAnimationInstance::GetDesiredFacing() const
{
	return Trajectory.GetSampleAtTime(0.5f, false).Facing;
}

void UGAFAnimationInstance::SetTraversalInteractionTransform_Implementation(const FTransform& InInteractionTransform)
{
	TraversalInteractionTransform = InInteractionTransform;
}

// 传入 Tag 找到 PoseHistory 节点并获取 PoseHistory
bool UGAFAnimationInstance::GetTraversalPoseHistoryReference_Implementation(
	const FName PoseHistoryTag,
	FPoseHistoryReference& OutPoseHistory)
{
	OutPoseHistory = FPoseHistoryReference{};

	if (PoseHistoryTag.IsNone())
	{
		UE_LOG(LogGAFAnimation, Warning, TEXT("%s failed to get traversal pose history: PoseHistoryTag is None. Check MotionMatchingSettings.TraversalPoseHistoryTag."), *GetNameSafe(this));

		ensureMsgf(false,
			TEXT("%s failed to get traversal pose history because PoseHistoryTag is None."),
			*GetNameSafe(this));
		return false;
	}

	// 底层会使用 TagSubsystem->FindNodeByTag<FAnimNode_PoseSearchHistoryCollector_Base> 通过 Tag 找到 PoseHistory 节点
	const FAnimNode_PoseSearchHistoryCollector_Base* PoseHistoryNode =
		UPoseSearchLibrary::FindPoseHistoryNode(PoseHistoryTag, this);

	if (PoseHistoryNode == nullptr)
	{
		UE_LOG(LogGAFAnimation, Warning, TEXT("%s failed to get traversal pose history: cannot find PoseSearchHistoryCollector node tagged [%s]. Check the AnimGraph node Tag."), *GetNameSafe(this), *PoseHistoryTag.ToString());

		ensureMsgf(false,
			TEXT("%s failed to find PoseSearchHistoryCollector node tagged [%s]."),
			*GetNameSafe(this),
			*PoseHistoryTag.ToString());
		return false;
	}

	if (PoseHistoryNode->GetPoseHistoryPtr() == nullptr)
	{
		UE_LOG(LogGAFAnimation, Warning, TEXT("%s failed to get traversal pose history: PoseSearchHistoryCollector node [%s] exists, but its PoseHistory is invalid."), *GetNameSafe(this), *PoseHistoryTag.ToString());

		ensureMsgf(false,
			TEXT("%s found PoseSearchHistoryCollector node [%s], but PoseHistory is invalid."),
			*GetNameSafe(this),
			*PoseHistoryTag.ToString());
		return false;
	}

	OutPoseHistory = PoseHistoryNode->GetPoseHistoryReference();
	return true;
}
