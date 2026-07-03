#include "Traversal/GAFCharacterTraversalComponent.h"

#include "GAFLogChannels.h"
#include "Traversal/GAFTraversalCollisionResolver.h"
#include "Traversal/GAFTraversableLedgeProviderComponent.h"
#include "Animation/AnimationTypes.h"
#include "DrawDebugHelpers.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFCharacterTraversalComponent)

UGAFCharacterTraversalComponent::UGAFCharacterTraversalComponent()
{
	// 翻越组件不需要Tick，每次翻越才会调用，状态也都是临时状态
	PrimaryComponentTick.bCanEverTick = false;
}

bool UGAFCharacterTraversalComponent::TryTraversalAction(
	const FGAFTraversalCheckInputs& TraversalCheckInputs,
	const FGAFTraversalSettings& TraversalSettings,
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

	// Step 1 : 通过接口获取翻越所需数据
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

	// Step 2.1 Traversable Object Search : 往角色前方向进行一次Trace，尝试找到含有 TraversableLedgeProvider 的物体
	// 如果找到了，就设置 Hit Component, 反之退出并返回失败原因
	
	// 2.1.1 起点 = 角色位置 + 起点偏移值
	const FVector TraceStart = ActorLocation + TraceOriginOffset;
	
	// 2.1.2 终点 = 起点 + 角色前向*Trace距离 + 终点偏移值
	const FVector TraceEnd =
		TraceStart
		+ TraceForwardDirection * TraceForwardDistance
		+ TraceEndOffset;
	
	// 2.1.3 使用 Capsule Trace
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

	// Step 2.2 Traversable Ledge Search : 如果检测到了可翻越物体，调用其内部函数找到 Front/Back Ledge
	bool bGetLedge = TraversableProvider->GetLedgeTransforms(TraversableSearchHitResult.ImpactPoint, ActorLocation, InOutTraversalCheckResult);
	InOutTraversalCheckResult.HitComponent = TraversableSearchHitResult.GetComponent();
	
	// DEBUG(Step2): 绘制前后边缘位置
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

	// Step 3.1 : If the traversable level block has a valid front ledge, continue the function. If not, exit early.
	if (!InOutTraversalCheckResult.bHasFrontLedge)
	{
		InOutTraversalCheckResult.FailureReason = EGAFTraversalFailureReason::CantFindFrontLedge;
		return false;
	}

	// Step 3.2 Has Room Check : 从角色位置往前边缘进行Trace，判断角色是否能够移动到攀爬点(是否有障碍物)
	
	// 3.2.1 终点 = FrontLedge位置:1. 向 LedgeNormal 偏移一个胶囊体半径 2. 向 Z 轴上方偏移一个胶囊体半高
	// 会加一个小的偏移量(2.0f)，LedgeNormal 固定为朝向可攀爬侧(角色进入方向)
	// TODO:这种检查办法应该局限于非常方正的可攀爬物
	const FVector HasRoomCheckFrontLedgeLocation = 
		InOutTraversalCheckResult.FrontLedgeLocation 
		+ InOutTraversalCheckResult.FrontLedgeNormal * (TraceCapsuleRadius + 2.0f)
		+ FVector{0.0f, 0.0f, TraceCapsuleHalfHeight + 2.0f};
	
	EffectiveDebugType = DebugDrawLevel >= 3 ? TraversalDebugType : EDrawDebugTrace::None;

	// 3.2.2 使用 Capsule Trace
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

	// Step 3.3: 障碍物高度 = FrontLedge 高度 - 角色胶囊底部高度
	const FVector ActorCapsuleBottomLocation =
		ActorLocation - FVector::ZAxisVector * TraceCapsuleHalfHeight;

	const float ObstacleHeight =
		FMath::Abs(InOutTraversalCheckResult.FrontLedgeLocation.Z - ActorCapsuleBottomLocation.Z);

	InOutTraversalCheckResult.ObstacleHeight = ObstacleHeight;

	// Step 3.4 Top Sweep : 如果存在 BackLedge，则从 FrontLedge 到 BackLedge 做一次顶部空间检测
	// 用来判断角色胶囊体是否能从障碍物顶部移动到另一侧
	if (InOutTraversalCheckResult.bHasBackLedge)
	{
		// 终点 = BackLedge位置: 1. 向 BackLedgeNormal 偏移一个胶囊体半径 2. 向 Z 轴上方偏移一个胶囊体半高
		// 和 FrontLedge 的 HasRoomCheck 位置保持一致，使顶部 Sweep 检测的是角色胶囊体中心的移动路径
		const FVector HasRoomCheckBackLedgeLocation =
			InOutTraversalCheckResult.BackLedgeLocation
			+ InOutTraversalCheckResult.BackLedgeNormal * (TraceCapsuleRadius + 2.0f)
			+ FVector::ZAxisVector * (TraceCapsuleHalfHeight + 2.0f);

		EffectiveDebugType = DebugDrawLevel >= 3 ? TraversalDebugType : EDrawDebugTrace::None;

		// 从 FrontLedge 的可站位点扫到 BackLedge 的可站位点，检测顶部是否有阻挡
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
			// Step 3.5A : 顶部没有阻挡，障碍物深度使用 FrontLedge 到 BackLedge 的水平距离
			InOutTraversalCheckResult.ObstacleDepth =
				(InOutTraversalCheckResult.BackLedgeLocation - InOutTraversalCheckResult.FrontLedgeLocation).Size2D();

			// Step 3.6 Back Floor Check : 从 BackLedge 上方往下扫，寻找另一侧可落脚地面
			// 起点使用 BackLedge 的可站位点，终点向下移动障碍物高度并额外多扫一段距离，避免轻微高度差导致漏检
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
				// 找到地面时记录地面位置，并计算 BackLedge 到地面的高度差
				InOutTraversalCheckResult.bHasBackFloor = true;
				InOutTraversalCheckResult.BackFloorLocation = BackFloorHitResult.Location;
				InOutTraversalCheckResult.BackLedgeHeight =
					FMath::Abs(InOutTraversalCheckResult.BackLedgeLocation.Z - BackFloorHitResult.Location.Z);
			}
			else
			{
				// 没有找到地面时让 BackFloor 失效，不判定 Traversal 失败
				InOutTraversalCheckResult.bHasBackFloor = false;
				InOutTraversalCheckResult.BackFloorLocation = FVector::ZeroVector;
				InOutTraversalCheckResult.BackLedgeHeight = 0.0f;
			}
		}
		else
		{
			// Step 3.5B : 顶部空间被挡住，障碍物深度使用 FrontLedge 到阻挡点的水平距离
			// 同时让 BackLedge 失效，后续逻辑会把它当作没有完整另一侧边缘的 Traversal 处理
			InOutTraversalCheckResult.ObstacleDepth =
				(TopSweepHitResult.ImpactPoint - InOutTraversalCheckResult.FrontLedgeLocation).Size2D();
			InOutTraversalCheckResult.bHasBackLedge = false;
			InOutTraversalCheckResult.bHasBackFloor = false;
		}
	}

	// Step 4.1: Send the front ledge location to the Anim BP using an interface.
	// This transform will be used for a custom channel within the following Motion Matching search.

	// Step 4.2: Evaluate a chooser with a PoseMatch Column to select the best montages that
	// match the conditions of the traversal check (Inside the Chooser, the Posematch Column is at the far right).
	// The chooser outputs the best montage with the right entry frame (start time) based on the distance to the ledge,
	// and the current characters pose(through anim instance). The Treversal Check struct is updated with this data and
	// will be used to play the montage through the PerformTraversalAction fucntion in Step: 5.5.

	// DEBUG: Print out the resulting conditions and parameters.

	// Step 5.1: Continue if there is a valid action type. If none of the conditions were met, no action can be performed, therefore exit the function.

	// Step 5.2: Finally, if the check was a success and a montage was found, trigger the Traversal Event
	return false;
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

ACharacter* UGAFCharacterTraversalComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}
