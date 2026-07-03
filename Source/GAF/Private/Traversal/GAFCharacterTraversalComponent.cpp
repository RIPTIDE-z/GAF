#include "Traversal/GAFCharacterTraversalComponent.h"

#include "GAFLogChannels.h"
#include "AnimInstance/GAFAnimInstanceDataProvider.h"
#include "Traversal/GAFTraversalCollisionResolver.h"
#include "Traversal/GAFTraversableLedgeProviderComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimationTypes.h"
#include "Chooser.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "IObjectChooser.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Math/RotationMatrix.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFCharacterTraversalComponent)

namespace
{
	// 当前使用 Switch 手动对应错误原因，减少反射解析的开销
	// TODO：换成更高效的框架
	const TCHAR* LexToString(const EGAFTraversalFailureReason FailureReason)
	{
		switch (FailureReason)
		{
			case EGAFTraversalFailureReason::None:
				return TEXT("None");
			case EGAFTraversalFailureReason::InvalidOwner:
				return TEXT("InvalidOwner");
			case EGAFTraversalFailureReason::InvalidMovementComponent:
				return TEXT("InvalidMovementComponent");
			case EGAFTraversalFailureReason::AlreadyDoingTraversal:
				return TEXT("AlreadyDoingTraversal");
			case EGAFTraversalFailureReason::CantFindTraversableObject:
				return TEXT("CantFindTraversableObject");
			case EGAFTraversalFailureReason::CantFindFrontLedge:
				return TEXT("CantFindFrontLedge");
			case EGAFTraversalFailureReason::NoRoomMoveToFrontLedge:
				return TEXT("NoRoomMoveToFrontLedge");
			case EGAFTraversalFailureReason::InvalidAnimInstance:
				return TEXT("InvalidAnimInstance");
			case EGAFTraversalFailureReason::InvalidPoseHistory:
				return TEXT("InvalidPoseHistory");
			case EGAFTraversalFailureReason::InvalidTraversalChooser:
				return TEXT("InvalidTraversalChooser");
			case EGAFTraversalFailureReason::TraversalCheckFailed:
				return TEXT("TraversalCheckFailed");
			case EGAFTraversalFailureReason::MontageSelectionFailed:
				return TEXT("MontageSelectionFailed");
			case EGAFTraversalFailureReason::InvalidTraversalMesh:
				return TEXT("InvalidTraversalMesh");
			case EGAFTraversalFailureReason::InvalidMotionWarpingComponent:
				return TEXT("InvalidMotionWarpingComponent");
			case EGAFTraversalFailureReason::InvalidTraversalMontage:
				return TEXT("InvalidTraversalMontage");
			case EGAFTraversalFailureReason::InvalidTraversalHitComponent:
				return TEXT("InvalidTraversalHitComponent");
			case EGAFTraversalFailureReason::MissingBackLedgeWarpWindow:
				return TEXT("MissingBackLedgeWarpWindow");
			case EGAFTraversalFailureReason::MissingBackFloorWarpWindow:
				return TEXT("MissingBackFloorWarpWindow");
			case EGAFTraversalFailureReason::MissingDistanceFromLedgeCurve:
				return TEXT("MissingDistanceFromLedgeCurve");
			case EGAFTraversalFailureReason::WarpTargetUpdateFailed:
				return TEXT("WarpTargetUpdateFailed");
			case EGAFTraversalFailureReason::MontagePlayFailed:
				return TEXT("MontagePlayFailed");
			default:
				return TEXT("Unknown");
		}
	}

	bool TryEvaluateMontageCurveAtTime(
		const UAnimMontage& Montage,
		const FName CurveName,
		const float MontageSampleTime,
		float& OutCurveValue)
	{
		if (Montage.SlotAnimTracks.Num() > 0)
		{
			const FAnimTrack& AnimTrack = Montage.SlotAnimTracks[0].AnimTrack;
			if (const FAnimSegment* Segment = AnimTrack.GetSegmentAtTime(MontageSampleTime))
			{
				const UAnimSequenceBase* AnimReference = Segment->GetAnimReference();
				if (IsValid(AnimReference) && AnimReference->HasCurveData(CurveName))
				{
					float AnimationSampleTime = Segment->ConvertTrackPosToAnimPos(MontageSampleTime);
					AnimationSampleTime = FMath::Clamp(AnimationSampleTime, Segment->AnimStartTime, Segment->AnimEndTime);

					const FAnimExtractContext CurveContext{ static_cast<double>(AnimationSampleTime) };
					OutCurveValue = AnimReference->EvaluateCurveData(CurveName, CurveContext);
					return true;
				}
			}
		}

		if (Montage.HasCurveData(CurveName))
		{
			const FAnimExtractContext CurveContext{ static_cast<double>(MontageSampleTime) };
			OutCurveValue = Montage.EvaluateCurveData(CurveName, CurveContext);
			return true;
		}

		return false;
	}
} // namespace

UGAFCharacterTraversalComponent::UGAFCharacterTraversalComponent()
{
	// 翻越组件不需要Tick，每次翻越才会调用，状态也都是临时状态
	PrimaryComponentTick.bCanEverTick = false;
}

ACharacter* UGAFCharacterTraversalComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}

bool UGAFCharacterTraversalComponent::TryTraversalAction(
	const FGAFTraversalCheckInputs& TraversalCheckInputs,
	const FGAFTraversalSettings& TraversalSettings,
	const FGAFMotionMatchingSettings& MotionMatchingSettings,
	const EDrawDebugTrace::Type TraversalDebugType,
	FGAFTraversalCheckResult& InOutTraversalCheckResult)
{

	ACharacter* Character = GetOwnerCharacter();
	if (!IsValid(Character))
	{
		InOutTraversalCheckResult.FailureReason = EGAFTraversalFailureReason::InvalidOwner;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to try traversal action: owner is not a valid Character."), *GetNameSafe(this));
		return false;
	}

	if (!IsValid(Character->GetCharacterMovement()))
	{
		InOutTraversalCheckResult.FailureReason = EGAFTraversalFailureReason::InvalidMovementComponent;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to try traversal action: CMC is invalid on [%s]."), *GetNameSafe(this), *GetNameSafe(Character));
		return false;
	}

	if (bDoingTraversalAction)
	{
		InOutTraversalCheckResult.FailureReason = EGAFTraversalFailureReason::AlreadyDoingTraversal;
		return false;
	}

	// 存放检测结果
	InOutTraversalCheckResult.Reset();

	// 优先使用角色配置覆盖的 TraceChannel，否则回退到项目级 TraversalConfig
	const ECollisionChannel TraversalTraceChannel = FGAFTraversalCollisionResolver::GetTraversalCollisionChannel(&TraversalSettings);

	// Step 1 : 通过角色数据接口获取 Traversal 所需的当前帧数据
	// 这里不直接依赖具体 Character 子类，而是通过 IGAFCharacterDataProvider 读取 Mesh、MovementMode、Gait、Speed 等数据
	CachedTraversalData = FGAFTraversalFrameData{};

	const IGAFCharacterDataProvider* Provider =
		Cast<IGAFCharacterDataProvider>(Character);
	const bool bHasData = Provider && Provider->GetTraversalFrameData(CachedTraversalData);

	// 数据无效
	// TODO:考虑使用上一帧数据而不是直接用默认值
	if (!bHasData)
	{
		CachedTraversalData = FGAFTraversalFrameData{};
	}

	// 可复用的Trace设定，结果缓存
	const FVector& ActorLocation = Character->GetActorLocation();
	const float& TraceCapsuleRadius = TraversalCheckInputs.TraceCapsuleRadius;
	const float& TraceCapsuleHalfHeight = TraversalCheckInputs.TraceCapsuleHalfHeight;
	const ETraceTypeQuery& TraceChannel = UEngineTypes::ConvertToTraceType(TraversalTraceChannel);
	const float& DebugDrawDuration = TraversalSettings.DebugDrawDuration;
	const int32& DebugDrawLevel = TraversalSettings.DebugDrawLevel;

	const FVector& TraceOriginOffset = TraversalCheckInputs.TraceOriginOffset;
	const FVector& TraceForwardDirection = TraversalCheckInputs.TraceForwardDirection;
	const float& TraceForwardDistance = TraversalCheckInputs.TraceForwardDistance;
	const FVector& TraceEndOffset = TraversalCheckInputs.TraceEndOffset;

	// 忽略自身
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Character);

	EDrawDebugTrace::Type EffectiveDebugType = DebugDrawLevel >= 2 ? TraversalDebugType : EDrawDebugTrace::None;

	// Step 2.1 : 搜索可翻越物体
	// 沿角色前方做一次胶囊体 Trace，命中的 Actor 必须挂有且只挂有一个 TraversableLedgeProvider
	// 找到有效 Provider 后，保存本次命中的 HitComponent，后续 MotionWarping 或播放逻辑可以继续使用

	// 2.1.1 : 起点 = 角色位置 + 起点偏移值
	const FVector TraceStart = ActorLocation + TraceOriginOffset;

	// 2.1.2 : 终点 = 起点 + 角色前向*Trace距离 + 终点偏移值
	const FVector TraceEnd =
		TraceStart
		+ TraceForwardDirection * TraceForwardDistance
		+ TraceEndOffset;

	// 2.1.3 : 使用 Capsule Trace
	FHitResult TraversableSearchHitResult;
	const bool bTraversableSearchHit = UKismetSystemLibrary::CapsuleTraceSingle(
		this,
		TraceStart,
		TraceEnd,
		TraceCapsuleRadius,
		TraceCapsuleHalfHeight,
		TraceChannel,
		false,
		ActorsToIgnore,
		EffectiveDebugType,
		TraversableSearchHitResult,
		true,
		FLinearColor::Black,
		FLinearColor::Black,
		DebugDrawDuration);

	if (!bTraversableSearchHit)
	{
		InOutTraversalCheckResult.FailureReason = EGAFTraversalFailureReason::CantFindTraversableObject;
		return false;
	}

	const AActor* HitActor = TraversableSearchHitResult.GetActor();
	if (!IsValid(HitActor))
	{
		InOutTraversalCheckResult.FailureReason = EGAFTraversalFailureReason::CantFindTraversableObject;
		return false;
	}

	TArray<UGAFTraversableLedgeProviderComponent*> TraversableProviders;
	HitActor->GetComponents<UGAFTraversableLedgeProviderComponent>(TraversableProviders);

	if (TraversableProviders.IsEmpty())
	{
		InOutTraversalCheckResult.FailureReason = EGAFTraversalFailureReason::CantFindTraversableObject;
		return false;
	}

	if (TraversableProviders.Num() > 1)
	{
		InOutTraversalCheckResult.FailureReason = EGAFTraversalFailureReason::CantFindTraversableObject;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to try traversal action: hit actor [%s] has multiple TraversableLedgeProvider components. Only one provider is allowed per traversable actor."), *GetNameSafe(this), *GetNameSafe(HitActor));
		return false;
	}

	const UGAFTraversableLedgeProviderComponent* TraversableProvider = TraversableProviders[0];
	if (!IsValid(TraversableProvider))
	{
		InOutTraversalCheckResult.FailureReason = EGAFTraversalFailureReason::CantFindTraversableObject;
		return false;
	}

	InOutTraversalCheckResult.HitComponent = TraversableSearchHitResult.GetComponent();

	// Step 2.2 : 搜索可翻越边缘
	// Provider 会根据命中位置和角色位置，在自己的 LedgePair 中选择 FrontLedge / BackLedge，并写入 CheckResult
	bool bGetLedge = TraversableProvider->GetLedgeTransforms(TraversableSearchHitResult.ImpactPoint, ActorLocation, InOutTraversalCheckResult);
	InOutTraversalCheckResult.HitComponent = TraversableSearchHitResult.GetComponent();

	// DEBUG : 绘制 Step 2 找到的前后边缘位置，绿色表示 FrontLedge，蓝色表示 BackLedge。
	// TODO:提取为单独模块
	if (TraversalSettings.DebugDrawLevel >= 1)
	{
		UWorld* World = GetWorld();
		if (IsValid(World))
		{
			if (InOutTraversalCheckResult.bHasFrontLedge)
			{
				DrawDebugSphere(
					World,
					InOutTraversalCheckResult.FrontLedgeLocation,
					10.0f,
					12,
					FColor::Green,
					false,
					DebugDrawDuration,
					0,
					1.0f);
			}

			if (InOutTraversalCheckResult.bHasBackLedge)
			{
				DrawDebugSphere(
					World,
					InOutTraversalCheckResult.BackLedgeLocation,
					10.0f,
					12,
					FColor::Blue,
					false,
					DebugDrawDuration,
					0,
					1.0f);
			}
		}
	}

	// Step 3.1 : FrontLedge 是 Traversal 的最低必要条件
	// 没有 FrontLedge 就无法确定角色要靠近的攀爬点，因此直接退出
	if (!InOutTraversalCheckResult.bHasFrontLedge)
	{
		InOutTraversalCheckResult.FailureReason = EGAFTraversalFailureReason::CantFindFrontLedge;
		return false;
	}

	// Step 3.2 : 检查角色是否有空间移动到 FrontLedge
	// 从角色当前位置扫到 FrontLedge 前方的胶囊体中心位置，如果中途有阻挡，说明角色不能进入攀爬起点

	// 3.2.1 : 计算角色靠近 FrontLedge 时胶囊体中心应该到达的位置
	// 位置 = FrontLedge 位置 + 朝向角色侧的法线偏移一个半径 + 向上偏移一个半高
	// 额外的 2.0f 是安全余量，避免刚好贴边时因为浮点误差被判定为重叠
	// TODO:这种检查办法应该局限于非常方正的可攀爬物
	const FVector HasRoomCheckFrontLedgeLocation =
		InOutTraversalCheckResult.FrontLedgeLocation
		+ InOutTraversalCheckResult.FrontLedgeNormal * (TraceCapsuleRadius + 2.0f)
		+ FVector{ 0.0f, 0.0f, TraceCapsuleHalfHeight + 2.0f };

	EffectiveDebugType = DebugDrawLevel >= 3 ? TraversalDebugType : EDrawDebugTrace::None;

	// 3.2.2 : 使用胶囊体 Sweep 检查从当前位置到攀爬起点之间是否有阻挡
	FHitResult HasRoomHitResult;
	const bool bHasRoomHit = UKismetSystemLibrary::CapsuleTraceSingle(
		this,
		ActorLocation,
		HasRoomCheckFrontLedgeLocation,
		TraceCapsuleRadius,
		TraceCapsuleHalfHeight,
		TraceChannel,
		false,
		ActorsToIgnore,
		EffectiveDebugType,
		HasRoomHitResult,
		true,
		FLinearColor::Black,
		FLinearColor::Black,
		DebugDrawDuration);

	// 有阻挡物，角色无法攀爬
	if (HasRoomHitResult.bBlockingHit || HasRoomHitResult.bStartPenetrating)
	{
		InOutTraversalCheckResult.FailureReason = EGAFTraversalFailureReason::NoRoomMoveToFrontLedge;
		return false;
	}

	// Step 3.3 : 计算障碍物高度
	// 高度 = FrontLedge 高度 - 当前角色胶囊底部高度，用于后续区分 Hurdle / Vault / Mantle 等动作条件
	const FVector ActorCapsuleBottomLocation =
		ActorLocation - FVector::ZAxisVector * TraceCapsuleHalfHeight;

	const float ObstacleHeight =
		FMath::Abs(InOutTraversalCheckResult.FrontLedgeLocation.Z - ActorCapsuleBottomLocation.Z);

	InOutTraversalCheckResult.ObstacleHeight = ObstacleHeight;

	// Step 3.4 : 顶部空间检测
	// 如果存在 BackLedge，就从 FrontLedge 的可站位点扫到 BackLedge 的可站位点
	// 这个检查用于判断角色胶囊体是否真的有空间越过障碍物顶部并移动到另一侧
	if (InOutTraversalCheckResult.bHasBackLedge)
	{
		// 计算角色到达 BackLedge 另一侧时胶囊体中心应该处在的位置
		// 它和 FrontLedge 的 HasRoomCheck 位置规则一致，使顶部 Sweep 检测的是胶囊体中心移动路径
		const FVector HasRoomCheckBackLedgeLocation =
			InOutTraversalCheckResult.BackLedgeLocation
			+ InOutTraversalCheckResult.BackLedgeNormal * (TraceCapsuleRadius + 2.0f)
			+ FVector::ZAxisVector * (TraceCapsuleHalfHeight + 2.0f);

		EffectiveDebugType = DebugDrawLevel >= 3 ? TraversalDebugType : EDrawDebugTrace::None;

		// 从 FrontLedge 可站位点扫到 BackLedge 可站位点，检测顶部路径是否被阻挡
		FHitResult TopSweepHitResult;
		const bool bTopSweepHit = UKismetSystemLibrary::CapsuleTraceSingle(
			this,
			HasRoomCheckFrontLedgeLocation,
			HasRoomCheckBackLedgeLocation,
			TraceCapsuleRadius,
			TraceCapsuleHalfHeight,
			TraceChannel,
			false,
			ActorsToIgnore,
			EffectiveDebugType,
			TopSweepHitResult,
			true,
			FLinearColor::Red,
			FLinearColor::Green,
			DebugDrawDuration);

		if (!bTopSweepHit)
		{
			// Step 3.5A : 顶部没有阻挡
			// 障碍物深度使用 FrontLedge 到 BackLedge 的水平距离，表示角色要越过的顶部跨度
			InOutTraversalCheckResult.ObstacleDepth =
				(InOutTraversalCheckResult.BackLedgeLocation - InOutTraversalCheckResult.FrontLedgeLocation).Size2D();

			// Step 3.6 : BackFloor 检测
			// 从 BackLedge 的可站位点向下扫，寻找另一侧可落脚地面
			// 扫描距离 = 障碍物高度 + 额外距离，用于覆盖另一侧地面略低于当前角色底部的情况
			const FVector BackFloorTraceEnd =
				HasRoomCheckBackLedgeLocation
				- FVector::ZAxisVector * (InOutTraversalCheckResult.ObstacleHeight + TraversalSettings.BackFloorTraceExtraDistance);

			FHitResult BackFloorHitResult;
			const bool bBackFloorTraceHit = UKismetSystemLibrary::CapsuleTraceSingle(
				this,
				HasRoomCheckBackLedgeLocation,
				BackFloorTraceEnd,
				TraceCapsuleRadius,
				TraceCapsuleHalfHeight,
				TraceChannel,
				false,
				ActorsToIgnore,
				EffectiveDebugType,
				BackFloorHitResult,
				true,
				FLinearColor::Red,
				FLinearColor::Green,
				DebugDrawDuration);

			if (bBackFloorTraceHit && BackFloorHitResult.bBlockingHit)
			{
				// 找到地面时记录落地点，并计算 BackLedge 到 BackFloor 的高度差
				// 这个高度会参与动作选择，例如区分落地式 Hurdle 和空中式 Vault
				InOutTraversalCheckResult.bHasBackFloor = true;
				InOutTraversalCheckResult.BackFloorLocation = BackFloorHitResult.Location;
				InOutTraversalCheckResult.BackLedgeHeight =
					FMath::Abs(InOutTraversalCheckResult.BackLedgeLocation.Z - BackFloorHitResult.Location.Z);
			}
			else
			{
				// 没有找到地面时只让 BackFloor 失效，不直接判定 Traversal 失败
				// Chooser 可以继续根据 bHasBackFloor=false 选择 Vault 或 Mantle 等动作
				InOutTraversalCheckResult.bHasBackFloor = false;
				InOutTraversalCheckResult.BackFloorLocation = FVector::ZeroVector;
				InOutTraversalCheckResult.BackLedgeHeight = 0.0f;
			}
		}
		else
		{
			// Step 3.5B：顶部空间被阻挡
			// 障碍物深度改用 FrontLedge 到阻挡点的水平距离，并让 BackLedge / BackFloor 失效
			// 后续 Chooser 会把它当作没有完整另一侧边缘的 Traversal 处理
			InOutTraversalCheckResult.ObstacleDepth =
				(TopSweepHitResult.ImpactPoint - InOutTraversalCheckResult.FrontLedgeLocation).Size2D();
			InOutTraversalCheckResult.bHasBackLedge = false;
			InOutTraversalCheckResult.bHasBackFloor = false;
		}
	}

	// Step 4.1 : 把交互目标写入 AnimInstance
	// Motion Matching 的自定义 Channel 需要知道角色要对齐到哪个 Ledge
	// 这里用 FrontLedgeLocation 作为位置，用 FrontLedgeNormal 构造朝向
	// 通过接口写入 AnimInstance 的 InteractionTransform
	USkeletalMeshComponent* Mesh = CachedTraversalData.Mesh;
	UAnimInstance* AnimInstance = IsValid(Mesh)
		? Mesh->GetAnimInstance()
		: nullptr;
	IGAFAnimInstanceDataProvider* AnimProvider = Cast<IGAFAnimInstanceDataProvider>(AnimInstance);

	if (!IsValid(AnimInstance) || AnimProvider == nullptr)
	{
		InOutTraversalCheckResult.FailureReason = EGAFTraversalFailureReason::InvalidAnimInstance;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to try traversal action: anim instance data provider is invalid on [%s]."), *GetNameSafe(this), *GetNameSafe(Character));
		return false;
	}

	// 这里构造的旋转值的的 Z 轴朝向 FrontLedgeNormal
	// TODO:更详细地解释为什么这里要这么写
	const FVector InteractionNormal =
		InOutTraversalCheckResult.FrontLedgeNormal.GetSafeNormal(UE_SMALL_NUMBER, FVector::UpVector);
	const FTransform InteractionTransform{
		FRotationMatrix::MakeFromZ(InteractionNormal).ToQuat(),
		InOutTraversalCheckResult.FrontLedgeLocation,
		FVector::OneVector
	};

	AnimProvider->SetTraversalInteractionTransform(InteractionTransform);

	// Step 4.2 : 评估 Traversal Chooser，选择最合适的 Montage 和起始时间
	// Chooser 会使用一列专门的 PoseMatch Column 进行一次临时 MotionMatch
	// ChooserInput 会包含检测结果、角色状态、到 ledge 的距离以及当前 PoseHistory
	// Chooser 会根据自定义Feature:里Ledge的最近距离输出最佳的进入时机写回 CheckResult
	// 供后续 Step 5 PerformTraversalAction 播放蒙太奇使用
	const UChooserTable* TraversalMontageChooser = MotionMatchingSettings.TraversalMontageChooser.Get();
	if (!IsValid(TraversalMontageChooser))
	{
		InOutTraversalCheckResult.FailureReason = EGAFTraversalFailureReason::InvalidTraversalChooser;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to try traversal action: traversal montage chooser is invalid on [%s]."), *GetNameSafe(this), *GetNameSafe(Character));
		return false;
	}

	FPoseHistoryReference PoseHistory;
	if (!AnimProvider->GetTraversalPoseHistoryReference(MotionMatchingSettings.TraversalPoseHistoryTag, PoseHistory))
	{
		InOutTraversalCheckResult.FailureReason = EGAFTraversalFailureReason::InvalidPoseHistory;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to try traversal action: pose history [%s] is invalid on [%s]."), *GetNameSafe(this), *MotionMatchingSettings.TraversalPoseHistoryTag.ToString(), *GetNameSafe(Character));
		return false;
	}

	FGAFTraversalChooserInputs ChooserInputs;
	ChooserInputs.ActionType = InOutTraversalCheckResult.ActionType;
	ChooserInputs.bHasFrontLedge = InOutTraversalCheckResult.bHasFrontLedge;
	ChooserInputs.bHasBackLedge = InOutTraversalCheckResult.bHasBackLedge;
	ChooserInputs.bHasBackFloor = InOutTraversalCheckResult.bHasBackFloor;
	ChooserInputs.ObstacleHeight = InOutTraversalCheckResult.ObstacleHeight;
	ChooserInputs.ObstacleDepth = InOutTraversalCheckResult.ObstacleDepth;
	ChooserInputs.BackLedgeHeight = InOutTraversalCheckResult.BackLedgeHeight;
	ChooserInputs.MovementMode = CachedTraversalData.MovementMode;
	ChooserInputs.Gait = CachedTraversalData.Gait;
	ChooserInputs.Speed = CachedTraversalData.Speed;
	ChooserInputs.PoseHistory = PoseHistory;

	// Mesh 距离 FrontLedge 的位置
	const FVector DistanceSourceLocation = IsValid(Mesh)
		? Mesh->GetComponentLocation()
		: ActorLocation;
	ChooserInputs.DistanceToLedge =
		FVector::Distance(DistanceSourceLocation, InOutTraversalCheckResult.FrontLedgeLocation);

	FGAFTraversalChooserOutputs ChooserOutputs;

	// 构造 Chooser Context
	// 第一个参数放 AnimInstance，让 Chooser 可以读取对象上下文
	FChooserEvaluationContext ChooserContext{ AnimInstance };
	// 第二个参数放 ChooserInputs，作为条件筛选和 PoseMatch 的输入
	ChooserContext.AddStructParam(ChooserInputs);
	// 第三个参数放 ChooserOutputs，并登记到 OutputArrays，让 Output Struct Column 可以把行数据写回这个结构
	ChooserContext.OutputArrays.Add({ static_cast<uint32>(ChooserContext.Params.Num()) });
	ChooserContext.AddStructParam(ChooserOutputs);

	UObject* ResultObject = nullptr;
	// EvaluateChooser 通过回调逐个返回命中的 Result Object
	// 这里捕获第一个 ResultObject 后返回 Stop，表示只需要第一个匹配结果
	// 同时 Chooser 的 Output Struct Column 会在评估过程中把 ActionType / MontageStartTime 等写入 ChooserOutputs
	const FObjectChooserBase::EIteratorStatus ChooserStatus = UChooserTable::EvaluateChooser(
		ChooserContext,
		TraversalMontageChooser,
		FObjectChooserBase::FObjectChooserIteratorCallback::CreateLambda(
			[&ResultObject](UObject* InResultObject) {
				ResultObject = InResultObject;
				return FObjectChooserBase::EIteratorStatus::Stop;
			}));

	// 当前流程不直接依赖 ChooserStatus 做分支
	// 成功与否最终由 Output Struct 和 ResultObject 是否能给出有效 Montage 判断
	// 这里显式丢弃返回值，避免编译器产生未使用变量警告
	(void)ChooserStatus;

	// 优先使用 Output Struct 中配置的 Montage
	// 如果 Chooser 没有通过 Output Struct 输出 Montage，则尝试把普通 Result Object 当作 Montage 使用
	UAnimMontage* SelectedMontage = ChooserOutputs.ChosenMontage.Get();
	if (!IsValid(SelectedMontage))
	{
		SelectedMontage = Cast<UAnimMontage>(ResultObject);
	}

	if (!IsValid(SelectedMontage))
	{
		InOutTraversalCheckResult.FailureReason = EGAFTraversalFailureReason::MontageSelectionFailed;
		return false;
	}

	// 如果 ActionType 无效则判定失败
	if (ChooserOutputs.ActionType == EGAFTraversalActionType::None)
	{
		InOutTraversalCheckResult.FailureReason = EGAFTraversalFailureReason::TraversalCheckFailed;
		return false;
	}

	InOutTraversalCheckResult.ActionType = ChooserOutputs.ActionType;
	InOutTraversalCheckResult.ChosenMontage = SelectedMontage;
	InOutTraversalCheckResult.StartTime = ChooserOutputs.MontageStartTime;
	InOutTraversalCheckResult.PlayRate = MotionMatchingSettings.DefaultTraversalPlayRate;

	// DEBUG : 打印最终检测条件和 Chooser 选择结果
	DebugPrintTraversalCheckResult(InOutTraversalCheckResult, TraversalSettings);

	// Step 5 : 实际播放 Montage / MotionWarping
	return PerformTraversalAction(InOutTraversalCheckResult, TraversalSettings);
}

bool UGAFCharacterTraversalComponent::PerformTraversalAction(
	FGAFTraversalCheckResult& InOutTraversalResult,
	const FGAFTraversalSettings& TraversalSettings)
{
	ACharacter* Character = GetOwnerCharacter();
	if (!IsValid(Character))
	{
		InOutTraversalResult.FailureReason = EGAFTraversalFailureReason::InvalidOwner;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to perform traversal action: owner is not a valid Character."), *GetNameSafe(this));
		return false;
	}

	UAnimMontage* ChosenMontage = InOutTraversalResult.ChosenMontage.Get();
	if (!IsValid(ChosenMontage))
	{
		InOutTraversalResult.FailureReason = EGAFTraversalFailureReason::InvalidTraversalMontage;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to perform traversal action: chosen montage is invalid on [%s]."), *GetNameSafe(this), *GetNameSafe(Character));
		return false;
	}

	if (!InOutTraversalResult.bHasFrontLedge)
	{
		InOutTraversalResult.FailureReason = EGAFTraversalFailureReason::CantFindFrontLedge;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to perform traversal action: traversal result has no FrontLedge on [%s]."), *GetNameSafe(this), *GetNameSafe(Character));
		return false;
	}

	USkeletalMeshComponent* Mesh = CachedTraversalData.Mesh;
	if (!IsValid(Mesh))
	{
		InOutTraversalResult.FailureReason = EGAFTraversalFailureReason::InvalidTraversalMesh;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to perform traversal action: mesh is invalid on [%s]."), *GetNameSafe(this), *GetNameSafe(Character));
		return false;
	}

	UAnimInstance* AnimInstance = IsValid(Mesh)
		? Mesh->GetAnimInstance()
		: nullptr;
	if (!IsValid(AnimInstance))
	{
		InOutTraversalResult.FailureReason = EGAFTraversalFailureReason::InvalidAnimInstance;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to perform traversal action: anim instance is invalid on [%s]."), *GetNameSafe(this), *GetNameSafe(Character));
		return false;
	}

	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	if (!IsValid(Movement))
	{
		InOutTraversalResult.FailureReason = EGAFTraversalFailureReason::InvalidMovementComponent;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to perform traversal action: CMC is invalid on [%s]."), *GetNameSafe(this), *GetNameSafe(Character));
		return false;
	}

	if (!IsValid(InOutTraversalResult.HitComponent.Get()))
	{
		InOutTraversalResult.FailureReason = EGAFTraversalFailureReason::InvalidTraversalHitComponent;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to perform traversal action: HitComponent is invalid on [%s]."), *GetNameSafe(this), *GetNameSafe(Character));
		return false;
	}

	// Step 5.1 : 播放选中的 Traversal Montage
	// StartTime 来自 Chooser / PoseMatch，用于从最匹配当前姿势的位置开始播放
	const float MontageDuration = AnimInstance->Montage_Play(
		ChosenMontage,
		InOutTraversalResult.PlayRate,
		EMontagePlayReturnType::Duration,
		InOutTraversalResult.StartTime);

	if (MontageDuration <= 0.0f)
	{
		InOutTraversalResult.FailureReason = EGAFTraversalFailureReason::MontagePlayFailed;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to perform traversal action: montage [%s] could not be played on [%s]."), *GetNameSafe(this), *GetNameSafe(ChosenMontage), *GetNameSafe(Character));
		return false;
	}

	// Step 5.2 : 绑定 Montage 回调
	// BlendingOut 覆盖蓝图 On Blend Out / On Interrupted
	// Ended 覆盖 On Completed / On Interrupted
	// 两个回调都走同一个收尾函数，FinishTraversalAction
	FOnMontageBlendingOutStarted BlendingOutDelegate;
	BlendingOutDelegate.BindUObject(this, &ThisClass::HandleTraversalMontageBlendingOut);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, ChosenMontage);

	FOnMontageEnded EndedDelegate;
	EndedDelegate.BindUObject(this, &ThisClass::HandleTraversalMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndedDelegate, ChosenMontage);

	// Step 5.3 : PlayMontage 后进入 Traversal 状态
	ActiveTraversalResult = InOutTraversalResult;
	bDoingTraversalAction = true;

	// Step 5.4 : 调用 UpdateWarpTargets 更新 MotionWarping 所需的 WarpTargets
	UpdateWarpTargets(InOutTraversalResult, TraversalSettings);

	// Step 5.5 : 应用执行期间的移动状态
	// 播放过程中忽略本次命中的障碍组件，并切到 Flying，避免 CharacterMovement 抢占 RootMotion 位移
	if (IsValid(CachedTraversalData.Capsule) && IsValid(InOutTraversalResult.HitComponent.Get()))
	{
		CachedTraversalData.Capsule->IgnoreComponentWhenMoving(InOutTraversalResult.HitComponent.Get(), true);
	}

	Movement->SetMovementMode(MOVE_Flying);

	return true;
}

void UGAFCharacterTraversalComponent::HandleTraversalMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	(void)Montage;
	(void)bInterrupted;

	// BlendOut 对应蓝图 Play Montage 的 On Blend Out / On Interrupted 分支
	// bInterrupted 为 true 时说明这次 BlendOut 是中断触发
	FinishTraversalAction();
}

void UGAFCharacterTraversalComponent::HandleTraversalMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	(void)Montage;
	(void)bInterrupted;

	// Ended 对应蓝图 Play Montage 的 On Completed / On Interrupted 分支
	// FinishTraversalAction 内部是幂等的，因此和 BlendOut 同时触发也不会重复恢复
	FinishTraversalAction();
}

void UGAFCharacterTraversalComponent::FinishTraversalAction()
{
	if (!bDoingTraversalAction)
	{
		return;
	}

	// Montage 回调进入收尾后立即解除 Traversal 状态
	bDoingTraversalAction = false;

	ACharacter* Character = GetOwnerCharacter();

	// 恢复 Traversal 期间忽略的障碍组件
	if (IsValid(CachedTraversalData.Capsule) && IsValid(ActiveTraversalResult.HitComponent.Get()))
	{
		CachedTraversalData.Capsule->IgnoreComponentWhenMoving(ActiveTraversalResult.HitComponent.Get(), false);
	}

	// 根据动作类型恢复 MovementMode，Vault 结束后保持 Falling，其余回到 Walking
	if (IsValid(Character))
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			EMovementMode TargetMovementMode{ MOVE_Walking };
			if (ActiveTraversalResult.ActionType == EGAFTraversalActionType::Vault)
			{
				TargetMovementMode = MOVE_Falling;
			}

			Movement->SetMovementMode(TargetMovementMode);
		}
	}

	ActiveTraversalResult.Reset();
}

// 使用 Motion Warping 组件让 Actor 移动到障碍物上的精确位置
// 使用边缘位置来更新该组件中的 Warp Target
void UGAFCharacterTraversalComponent::UpdateWarpTargets(
	const FGAFTraversalCheckResult& TraversalResult,
	const FGAFTraversalSettings& TraversalSettings)
{
	ACharacter* Character = GetOwnerCharacter();
	if (!IsValid(Character))
	{
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to update warp targets: owner is not a valid Character."), *GetNameSafe(this));
		return;
	}

	UMotionWarpingComponent* MotionWarping = CachedTraversalData.MotionWarping;
	if (!IsValid(MotionWarping))
	{
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to update warp targets: MotionWarpingComponent is invalid on [%s]."), *GetNameSafe(this), *GetNameSafe(Character));
		return;
	}

	UAnimMontage* ChosenMontage = TraversalResult.ChosenMontage.Get();
	if (!IsValid(ChosenMontage))
	{
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to update warp targets: chosen montage is invalid on [%s]."), *GetNameSafe(this), *GetNameSafe(Character));
		return;
	}

	if (!TraversalResult.bHasFrontLedge)
	{
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to update warp targets: traversal result has no FrontLedge on [%s]."), *GetNameSafe(this), *GetNameSafe(Character));
		return;
	}

	float AnimatedDistanceFromFrontLedgeToBackLedge{ 0.0f };

	// 分Front BackLedge BackFloor 进行更新
	UpdateFrontLedgeWarpTarget(*MotionWarping, TraversalResult, TraversalSettings, *Character);

	const bool bHasBackLedgeAnimatedDistance = UpdateBackLedgeWarpTarget(
		*MotionWarping,
		TraversalResult,
		TraversalSettings,
		*ChosenMontage,
		AnimatedDistanceFromFrontLedgeToBackLedge);

	UpdateBackFloorWarpTarget(
		*MotionWarping,
		TraversalResult,
		TraversalSettings,
		*ChosenMontage,
		bHasBackLedgeAnimatedDistance,
		AnimatedDistanceFromFrontLedgeToBackLedge);
}

void UGAFCharacterTraversalComponent::UpdateFrontLedgeWarpTarget(
	UMotionWarpingComponent& MotionWarping,
	const FGAFTraversalCheckResult& TraversalResult,
	const FGAFTraversalSettings& TraversalSettings,
	const ACharacter& Character) const
{
	// FrontLedge 是必需目标，位置轻微上抬，旋转用 -FrontLedgeNormal 作为 X 轴让角色面向边缘
	const FVector FrontLedgeNormal = TraversalResult.FrontLedgeNormal.GetSafeNormal(
		UE_SMALL_NUMBER,
		Character.GetActorForwardVector());
	const FVector FrontLedgeTargetLocation =
		TraversalResult.FrontLedgeLocation
		+ FVector{ 0.0f, 0.0f, 0.5f };
	const FRotator FrontLedgeTargetRotation =
		FRotationMatrix::MakeFromX(-FrontLedgeNormal).Rotator();

	MotionWarping.AddOrUpdateWarpTargetFromLocationAndRotation(
		TraversalSettings.FrontLedgeWarpTargetName,
		FrontLedgeTargetLocation,
		FrontLedgeTargetRotation);
}

bool UGAFCharacterTraversalComponent::UpdateBackLedgeWarpTarget(
	UMotionWarpingComponent& MotionWarping,
	const FGAFTraversalCheckResult& TraversalResult,
	const FGAFTraversalSettings& TraversalSettings,
	const UAnimMontage& ChosenMontage,
	float& OutAnimatedDistanceFromFrontLedgeToBackLedge) const
{
	const bool bShouldUpdateBackLedge =
		TraversalResult.bHasBackLedge
		&& (TraversalResult.ActionType == EGAFTraversalActionType::Hurdle
			|| TraversalResult.ActionType == EGAFTraversalActionType::Vault);

	if (!bShouldUpdateBackLedge)
	{
		MotionWarping.RemoveWarpTarget(TraversalSettings.BackLedgeWarpTargetName);
		return false;
	}

	// 需要在动画里有 WarpWindows 才能进行 Warp
	TArray<FMotionWarpingWindowData> BackLedgeWarpWindows;
	UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(
		&ChosenMontage,
		TraversalSettings.BackLedgeWarpTargetName,
		BackLedgeWarpWindows);

	if (BackLedgeWarpWindows.IsEmpty())
	{
		MotionWarping.RemoveWarpTarget(TraversalSettings.BackLedgeWarpTargetName);
		return false;
	}

	// 因为不同的 Traversal 动画会移动不同的距离，所以需要知道动画实际移动了多远
	// 这样才能正确地对它进行 Motion Warping。这里会在 Back Ledge Warp Window 的结束时刻缓存一个曲线值
	// 用来判断当角色在动画中到达 Back Ledge 位置时，动画距离 Front Ledge 有多远
	const float BackLedgeCurveSampleTime = BackLedgeWarpWindows[0].EndTime;
	const bool bHasCurveData = TryEvaluateMontageCurveAtTime(
		ChosenMontage,
		TraversalSettings.DistanceFromLedgeCurveName,
		BackLedgeCurveSampleTime,
		OutAnimatedDistanceFromFrontLedgeToBackLedge);

	if (!bHasCurveData)
	{
		MotionWarping.RemoveWarpTarget(TraversalSettings.BackLedgeWarpTargetName);
		return false;
	}

	MotionWarping.AddOrUpdateWarpTargetFromLocationAndRotation(
		TraversalSettings.BackLedgeWarpTargetName,
		TraversalResult.BackLedgeLocation,
		FRotator::ZeroRotator);

	return true;
}

void UGAFCharacterTraversalComponent::UpdateBackFloorWarpTarget(
	UMotionWarpingComponent& MotionWarping,
	const FGAFTraversalCheckResult& TraversalResult,
	const FGAFTraversalSettings& TraversalSettings,
	const UAnimMontage& ChosenMontage,
	bool bHasAnimatedDistanceFromFrontLedgeToBackLedge,
	float AnimatedDistanceFromFrontLedgeToBackLedge) const
{
	const bool bShouldUpdateBackFloor =
		TraversalResult.ActionType == EGAFTraversalActionType::Hurdle
		&& TraversalResult.bHasBackFloor
		&& TraversalResult.bHasBackLedge
		&& bHasAnimatedDistanceFromFrontLedgeToBackLedge;

	if (!bShouldUpdateBackFloor)
	{
		MotionWarping.RemoveWarpTarget(TraversalSettings.BackFloorWarpTargetName);
		return;
	}

	TArray<FMotionWarpingWindowData> BackFloorWarpWindows;
	UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(
		&ChosenMontage,
		TraversalSettings.BackFloorWarpTargetName,
		BackFloorWarpWindows);

	if (BackFloorWarpWindows.IsEmpty())
	{
		MotionWarping.RemoveWarpTarget(TraversalSettings.BackFloorWarpTargetName);
		return;
	}

	// BackFloor Warp Window 结束时的距离曲线表示动画接触地面时距离 FrontLedge 有多远
	float AnimatedDistanceFromFrontLedgeToBackFloor{ 0.0f };
	const float BackFloorCurveSampleTime = BackFloorWarpWindows[0].EndTime;
	bool bHasCurveData = TryEvaluateMontageCurveAtTime(
		ChosenMontage,
		TraversalSettings.DistanceFromLedgeCurveName,
		BackFloorCurveSampleTime,
		AnimatedDistanceFromFrontLedgeToBackFloor);

	if (!bHasCurveData)
	{
		MotionWarping.RemoveWarpTarget(TraversalSettings.BackFloorWarpTargetName);
		return;
	}

	// 由于不同动画可能会在距离不同的位置落到地面上(例如跑步跨越可能比走路或站立跨越移动得更远)
	// 因此这里使用动画中相对 Back Ledge 的总移动距离，作为 BackFloor warp point 的 X/Y 平面位置
	// 从技术上说，如果地面不是平的，或者路径中间有障碍物，这种做法可能会导致一些碰撞问题
	// TODO:如果所有 Traversal 动画都能使用固定的度量标准，会是一个更好的改进方向
	const float BackFloorHorizontalOffset =
		FMath::Abs(AnimatedDistanceFromFrontLedgeToBackLedge - AnimatedDistanceFromFrontLedgeToBackFloor);
	const FVector BackFloorHorizontalLocation =
		TraversalResult.BackLedgeLocation
		+ TraversalResult.BackLedgeNormal.GetSafeNormal(UE_SMALL_NUMBER, FVector::ForwardVector)
			* BackFloorHorizontalOffset;
	const FVector BackFloorTargetLocation{
		BackFloorHorizontalLocation.X,
		BackFloorHorizontalLocation.Y,
		TraversalResult.BackFloorLocation.Z
	};

	MotionWarping.AddOrUpdateWarpTargetFromLocationAndRotation(
		TraversalSettings.BackFloorWarpTargetName,
		BackFloorTargetLocation,
		FRotator::ZeroRotator);
}

void UGAFCharacterTraversalComponent::DebugPrintTraversalFailureReason(
	const FGAFTraversalCheckResult& TraversalCheckResult,
	const FGAFTraversalSettings& TraversalSettings) const
{
	if (TraversalSettings.DebugDrawLevel < 1)
	{
		return;
	}

	// 将 FailureReason 从枚举转为文本
	const FString DebugString = FString::Printf(
		TEXT("%s failed to Traverse. FailureReason: %s."),
		*GetNameSafe(GetOwner()),
		LexToString(TraversalCheckResult.FailureReason));

	UKismetSystemLibrary::PrintString(
		this,
		DebugString,
		true,
		false,
		TraversalSettings.DebugPrintColor,
		TraversalSettings.DebugPrintDuration,
		FName(TEXT("TraversalFailureReason")));
}

// 打印 Result 结果
void UGAFCharacterTraversalComponent::DebugPrintTraversalCheckResult(
	const FGAFTraversalCheckResult& TraversalCheckResult,
	const FGAFTraversalSettings& TraversalSettings) const
{
	if (TraversalSettings.DebugDrawLevel < 1)
	{
		return;
	}

	const FString ConditionDebugString = FString::Printf(
		TEXT("Has Front Ledge: %s\nHas Back Ledge: %s\nHas Back Floor: %s\nObstacle Height: %.2f\nObstacle Depth: %.2f\nBack Ledge Height: %.2f"),
		TraversalCheckResult.bHasFrontLedge ? TEXT("true") : TEXT("false"),
		TraversalCheckResult.bHasBackLedge ? TEXT("true") : TEXT("false"),
		TraversalCheckResult.bHasBackFloor ? TEXT("true") : TEXT("false"),
		TraversalCheckResult.ObstacleHeight,
		TraversalCheckResult.ObstacleDepth,
		TraversalCheckResult.BackLedgeHeight);

	UKismetSystemLibrary::PrintString(
		this,
		ConditionDebugString,
		true,
		false,
		TraversalSettings.DebugPrintColor,
		TraversalSettings.DebugPrintDuration,
		FName(TEXT("TraversalCheckResult")));

	const UEnum* ActionTypeEnum = StaticEnum<EGAFTraversalActionType>();
	const FString ActionTypeString = IsValid(ActionTypeEnum)
		? ActionTypeEnum->GetNameStringByValue(static_cast<int64>(TraversalCheckResult.ActionType))
		: TEXT("Unknown");

	const FString ChooserDebugString = FString::Printf(
		TEXT("Action Type: %s\nChosen Montage: %s\nStart Time: %.2f\nPlay Rate: %.2f"),
		*ActionTypeString,
		*GetNameSafe(TraversalCheckResult.ChosenMontage.Get()),
		TraversalCheckResult.StartTime,
		TraversalCheckResult.PlayRate);

	UKismetSystemLibrary::PrintString(
		this,
		ChooserDebugString,
		true,
		false,
		TraversalSettings.DebugPrintColor,
		TraversalSettings.DebugPrintDuration,
		FName(TEXT("TraversalChooserResult")));
}
