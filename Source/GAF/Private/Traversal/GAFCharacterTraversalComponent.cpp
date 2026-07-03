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
	
	const FVector& TraceOriginOffset =  TraversalCheckInputs.TraceOriginOffset;
	const FVector& TraceForwardDirection =  TraversalCheckInputs.TraceForwardDirection;
	const float& TraceForwardDistance =  TraversalCheckInputs.TraceForwardDistance;
	const FVector& TraceEndOffset =  TraversalCheckInputs.TraceEndOffset;

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
		UE_LOG(LogGAFTraversal, Warning,
			TEXT("%s failed to try traversal action: hit actor [%s] has multiple TraversableLedgeProvider components. Only one provider is allowed per traversable actor."),
			*GetNameSafe(this),
			*GetNameSafe(HitActor));
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
		+ FVector{0.0f, 0.0f, TraceCapsuleHalfHeight + 2.0f};
	
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
		UE_LOG(LogGAFTraversal, Warning,
			TEXT("%s failed to try traversal action: anim instance data provider is invalid on [%s]."),
			*GetNameSafe(this),
			*GetNameSafe(Character));
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
		UE_LOG(LogGAFTraversal, Warning,
			TEXT("%s failed to try traversal action: traversal montage chooser is invalid on [%s]."),
			*GetNameSafe(this),
			*GetNameSafe(Character));
		return false;
	}

	FPoseHistoryReference PoseHistory;
	if (!AnimProvider->GetTraversalPoseHistoryReference(MotionMatchingSettings.TraversalPoseHistoryTag, PoseHistory))
	{
		InOutTraversalCheckResult.FailureReason = EGAFTraversalFailureReason::InvalidPoseHistory;
		UE_LOG(LogGAFTraversal, Warning,
			TEXT("%s failed to try traversal action: pose history [%s] is invalid on [%s]."),
			*GetNameSafe(this),
			*MotionMatchingSettings.TraversalPoseHistoryTag.ToString(),
			*GetNameSafe(Character));
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
			[&ResultObject](UObject* InResultObject)
			{
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

	// Step 5.1 : 实际播放 Montage / MotionWarping 
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

	// Step 5.1 : 缓存本次执行结果并进入 Traversal 状态
	// 后续 Montage 回调会使用 ActiveTraversalResult 恢复碰撞和 MovementMode
	ActiveTraversalResult = InOutTraversalResult;
	bDoingTraversalAction = true;

	// Step 5.2 : 更新 MotionWarping 所需的 WarpTarget
	// 这一步只负责把检测结果转换成 MotionWarping 目标，不负责播放 Montage
	if (!UpdateWarpTargets(InOutTraversalResult, TraversalSettings))
	{
		if (InOutTraversalResult.FailureReason == EGAFTraversalFailureReason::None)
		{
			InOutTraversalResult.FailureReason = EGAFTraversalFailureReason::WarpTargetUpdateFailed;
		}

		FinishTraversalAction();
		return false;
	}

	// Step 5.3 : 播放选中的 Traversal Montage
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
		FinishTraversalAction();
		return false;
	}

	// Step 5.4 : 绑定 Montage 回调
	// BlendOut / Ended 都会走组件自己的收尾逻辑，保证任意结束路径都能恢复状态
	FOnMontageBlendingOutStarted BlendingOutDelegate;
	BlendingOutDelegate.BindUObject(this, &ThisClass::HandleTraversalMontageBlendingOut);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, ChosenMontage);

	FOnMontageEnded EndedDelegate;
	EndedDelegate.BindUObject(this, &ThisClass::HandleTraversalMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndedDelegate, ChosenMontage);

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

	ACharacter* Character = GetOwnerCharacter();

	// 取消 Traversal 期间对命中组件的移动忽略
	if (IsValid(CachedTraversalData.Capsule) && IsValid(ActiveTraversalResult.HitComponent.Get()))
	{
		CachedTraversalData.Capsule->IgnoreComponentWhenMoving(ActiveTraversalResult.HitComponent.Get(), false);
	}

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

	bDoingTraversalAction = false;
	ActiveTraversalResult.Reset();
}

// In order for the actor to move to the exact points on the obstacle, 
// we use a Motion Warping component which warps the montage’s root motion using notify states on the montage. 
// This function updates the warp targets in the component using the ledge locations.
bool UGAFCharacterTraversalComponent::UpdateWarpTargets(
	FGAFTraversalCheckResult& InOutTraversalResult,
	const FGAFTraversalSettings& TraversalSettings)
{
	ACharacter* Character = GetOwnerCharacter();
	if (!IsValid(Character))
	{
		InOutTraversalResult.FailureReason = EGAFTraversalFailureReason::InvalidOwner;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to update warp targets: owner is not a valid Character."), *GetNameSafe(this));
		return false;
	}

	UMotionWarpingComponent* MotionWarping = CachedTraversalData.MotionWarping;
	if (!IsValid(MotionWarping))
	{
		InOutTraversalResult.FailureReason = EGAFTraversalFailureReason::InvalidMotionWarpingComponent;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to update warp targets: MotionWarpingComponent is invalid on [%s]."), *GetNameSafe(this), *GetNameSafe(Character));
		return false;
	}

	UAnimMontage* ChosenMontage = InOutTraversalResult.ChosenMontage.Get();
	if (!IsValid(ChosenMontage))
	{
		InOutTraversalResult.FailureReason = EGAFTraversalFailureReason::InvalidTraversalMontage;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to update warp targets: chosen montage is invalid on [%s]."), *GetNameSafe(this), *GetNameSafe(Character));
		return false;
	}

	if (!InOutTraversalResult.bHasFrontLedge)
	{
		InOutTraversalResult.FailureReason = EGAFTraversalFailureReason::CantFindFrontLedge;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to update warp targets: traversal result has no FrontLedge on [%s]."), *GetNameSafe(this), *GetNameSafe(Character));
		return false;
	}

	// Step 1 : Update the FrontLedge warp target using the front ledge's location and rotation.
	// FrontLedge 是所有 Traversal 动作都会使用的主要对齐点
	// 位置使用 FrontLedgeLocation 并略微上抬 0.5，旋转使用 -FrontLedgeNormal 作为 X 轴，让角色面向边缘
	const FVector FrontLedgeNormal = InOutTraversalResult.FrontLedgeNormal.GetSafeNormal(UE_SMALL_NUMBER, Character->GetActorForwardVector());
	const FVector FrontLedgeTargetLocation =
		InOutTraversalResult.FrontLedgeLocation
		+ FVector{ 0.0f, 0.0f, 0.5f };
	const FRotator FrontLedgeTargetRotation =
		FRotationMatrix::MakeFromX(-FrontLedgeNormal).Rotator();

	MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(
		TraversalSettings.FrontLedgeWarpTargetName,
		FrontLedgeTargetLocation,
		FrontLedgeTargetRotation);

	// Step 2 : If the action type was a hurdle or a vault, we need to also update the BackLedge target. If it is not a hurdle or vault, remove it.
	float AnimatedDistanceFromFrontLedgeToBackLedge{ 0.0f };
	bool bShouldUseBackLedgeWarpTarget =
		InOutTraversalResult.bHasBackLedge
		&& (InOutTraversalResult.ActionType == EGAFTraversalActionType::Hurdle
			|| InOutTraversalResult.ActionType == EGAFTraversalActionType::Vault);

	if (bShouldUseBackLedgeWarpTarget)
	{
		TArray<FMotionWarpingWindowData> BackLedgeWarpWindows;
		UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(
			ChosenMontage,
			TraversalSettings.BackLedgeWarpTargetName,
			BackLedgeWarpWindows);

		bShouldUseBackLedgeWarpTarget = false;

		// Step 3 : Because the traversal animations move at different distances (no fixed metrics), we need to know how far the animation moves 
		// in order to warp it properly. Here we cache a curve value at the end of the Back Ledge warp window to 
		// determine how far the animation is from the front ledge once the character reaches the back ledge location in the animation.
		if (BackLedgeWarpWindows.IsEmpty())
		{
			InOutTraversalResult.FailureReason = EGAFTraversalFailureReason::MissingBackLedgeWarpWindow;
			UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to update warp targets: montage [%s] has no BackLedge warp window [%s]."), *GetNameSafe(this), *GetNameSafe(ChosenMontage), *TraversalSettings.BackLedgeWarpTargetName.ToString());
			return false;
		}

		const float BackLedgeCurveSampleTime = BackLedgeWarpWindows[0].EndTime;
		if (ChosenMontage->SlotAnimTracks.Num() > 0)
		{
			const FAnimTrack& AnimTrack = ChosenMontage->SlotAnimTracks[0].AnimTrack;
			if (const FAnimSegment* Segment = AnimTrack.GetSegmentAtTime(BackLedgeCurveSampleTime))
			{
				const UAnimSequenceBase* AnimReference = Segment->GetAnimReference();
				if (IsValid(AnimReference) && AnimReference->HasCurveData(TraversalSettings.DistanceFromLedgeCurveName))
				{
					float AnimationSampleTime = Segment->ConvertTrackPosToAnimPos(BackLedgeCurveSampleTime);
					AnimationSampleTime = FMath::Clamp(AnimationSampleTime, Segment->AnimStartTime, Segment->AnimEndTime);

					const FAnimExtractContext CurveContext{ static_cast<double>(AnimationSampleTime) };
					AnimatedDistanceFromFrontLedgeToBackLedge =
						AnimReference->EvaluateCurveData(TraversalSettings.DistanceFromLedgeCurveName, CurveContext);
					bShouldUseBackLedgeWarpTarget = true;
				}
			}
		}

		if (!bShouldUseBackLedgeWarpTarget && ChosenMontage->HasCurveData(TraversalSettings.DistanceFromLedgeCurveName))
		{
			const FAnimExtractContext CurveContext{ static_cast<double>(BackLedgeCurveSampleTime) };
			AnimatedDistanceFromFrontLedgeToBackLedge =
				ChosenMontage->EvaluateCurveData(TraversalSettings.DistanceFromLedgeCurveName, CurveContext);
			bShouldUseBackLedgeWarpTarget = true;
		}

		if (!bShouldUseBackLedgeWarpTarget)
		{
			InOutTraversalResult.FailureReason = EGAFTraversalFailureReason::MissingDistanceFromLedgeCurve;
			UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to update warp targets: montage [%s] has no distance curve [%s] for BackLedge warp target."), *GetNameSafe(this), *GetNameSafe(ChosenMontage), *TraversalSettings.DistanceFromLedgeCurveName.ToString());
			return false;
		}
	}

	// Step 4 : Update the BackLedge warp target.
	if (bShouldUseBackLedgeWarpTarget)
	{
		MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(
			TraversalSettings.BackLedgeWarpTargetName,
			InOutTraversalResult.BackLedgeLocation,
			FRotator::ZeroRotator);
	}
	else
	{
		MotionWarping->RemoveWarpTarget(TraversalSettings.BackLedgeWarpTargetName);
	}

	// Step 5 : If the action type was a hurdle, we need to also update the BackFloor target. If it is not a hurdle, remove it.
	float AnimatedDistanceFromFrontLedgeToBackFloor{ 0.0f };
	bool bShouldUseBackFloorWarpTarget =
		InOutTraversalResult.bHasBackFloor
		&& InOutTraversalResult.ActionType == EGAFTraversalActionType::Hurdle;

	if (bShouldUseBackFloorWarpTarget)
	{
		TArray<FMotionWarpingWindowData> BackFloorWarpWindows;
		UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(
			ChosenMontage,
			TraversalSettings.BackFloorWarpTargetName,
			BackFloorWarpWindows);

		bShouldUseBackFloorWarpTarget = false;

		// Step 6 : Caches a curve value at the end of the Back Floor warp window to determine 
		// how far the animation is from the front ledge once the character touches the ground.
		if (BackFloorWarpWindows.IsEmpty())
		{
			InOutTraversalResult.FailureReason = EGAFTraversalFailureReason::MissingBackFloorWarpWindow;
			UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to update warp targets: montage [%s] has no BackFloor warp window [%s]."), *GetNameSafe(this), *GetNameSafe(ChosenMontage), *TraversalSettings.BackFloorWarpTargetName.ToString());
			return false;
		}

		const float BackFloorCurveSampleTime = BackFloorWarpWindows[0].EndTime;
		if (ChosenMontage->SlotAnimTracks.Num() > 0)
		{
			const FAnimTrack& AnimTrack = ChosenMontage->SlotAnimTracks[0].AnimTrack;
			if (const FAnimSegment* Segment = AnimTrack.GetSegmentAtTime(BackFloorCurveSampleTime))
			{
				const UAnimSequenceBase* AnimReference = Segment->GetAnimReference();
				if (IsValid(AnimReference) && AnimReference->HasCurveData(TraversalSettings.DistanceFromLedgeCurveName))
				{
					float AnimationSampleTime = Segment->ConvertTrackPosToAnimPos(BackFloorCurveSampleTime);
					AnimationSampleTime = FMath::Clamp(AnimationSampleTime, Segment->AnimStartTime, Segment->AnimEndTime);

					const FAnimExtractContext CurveContext{ static_cast<double>(AnimationSampleTime) };
					AnimatedDistanceFromFrontLedgeToBackFloor =
						AnimReference->EvaluateCurveData(TraversalSettings.DistanceFromLedgeCurveName, CurveContext);
					bShouldUseBackFloorWarpTarget = true;
				}
			}
		}

		if (!bShouldUseBackFloorWarpTarget && ChosenMontage->HasCurveData(TraversalSettings.DistanceFromLedgeCurveName))
		{
			const FAnimExtractContext CurveContext{ static_cast<double>(BackFloorCurveSampleTime) };
			AnimatedDistanceFromFrontLedgeToBackFloor =
				ChosenMontage->EvaluateCurveData(TraversalSettings.DistanceFromLedgeCurveName, CurveContext);
			bShouldUseBackFloorWarpTarget = true;
		}

		if (!bShouldUseBackFloorWarpTarget)
		{
			InOutTraversalResult.FailureReason = EGAFTraversalFailureReason::MissingDistanceFromLedgeCurve;
			UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to update warp targets: montage [%s] has no distance curve [%s] for BackFloor warp target."), *GetNameSafe(this), *GetNameSafe(ChosenMontage), *TraversalSettings.DistanceFromLedgeCurveName.ToString());
			return false;
		}
	}

	// Step 7 : Since the animations may land on the floor at different distances (a run hurdle may travel further than a walk or stand hurdle), 
	// use the total animated distance away from the back ledge as the X and Y values of the BackFloor warp point. 
	// This could technically cause some collision issues if the floor is not flat, or there is an bostacle in the way, 
	// therefore having fixed metrics for all traversal animations would be an improvement.
	if (bShouldUseBackFloorWarpTarget)
	{
		const float BackFloorHorizontalOffset =
			FMath::Abs(AnimatedDistanceFromFrontLedgeToBackLedge - AnimatedDistanceFromFrontLedgeToBackFloor);
		const FVector BackFloorHorizontalLocation =
			InOutTraversalResult.BackLedgeLocation
			+ InOutTraversalResult.BackLedgeNormal.GetSafeNormal(UE_SMALL_NUMBER, FVector::ForwardVector)
			* BackFloorHorizontalOffset;
		const FVector BackFloorTargetLocation{
			BackFloorHorizontalLocation.X,
			BackFloorHorizontalLocation.Y,
			InOutTraversalResult.BackFloorLocation.Z
		};

		MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(
			TraversalSettings.BackFloorWarpTargetName,
			BackFloorTargetLocation,
			FRotator::ZeroRotator);
	}
	else
	{
		MotionWarping->RemoveWarpTarget(TraversalSettings.BackFloorWarpTargetName);
	}

	return true;
}

void UGAFCharacterTraversalComponent::DebugPrintTraversalFailureReason(
	const FGAFTraversalCheckResult& TraversalCheckResult,
	const FGAFTraversalSettings& TraversalSettings) const
{
	if (TraversalSettings.DebugDrawLevel < 1)
	{
		return;
	}

	const UEnum* FailureReasonEnum = StaticEnum<EGAFTraversalFailureReason>();
	const FString FailureReasonString = IsValid(FailureReasonEnum)
		? FailureReasonEnum->GetNameStringByValue(static_cast<int64>(TraversalCheckResult.FailureReason))
		: TEXT("Unknown");

	const FString DebugString = FString::Printf(
		TEXT("%s failed to Traverse. FailureReason: %s."),
		*GetNameSafe(GetOwner()),
		*FailureReasonString);

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
