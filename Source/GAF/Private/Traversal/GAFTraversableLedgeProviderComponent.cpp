#include "Traversal/GAFTraversableLedgeProviderComponent.h"

#include "GAFLogChannels.h"
#include "Traversal/GAFTraversableLedgeSplineComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFTraversableLedgeProviderComponent)

UGAFTraversableLedgeProviderComponent::UGAFTraversableLedgeProviderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGAFTraversableLedgeProviderComponent::BeginPlay()
{
	Super::BeginPlay();

	// 在 BeginPlay 统一解析编辑器配置，后续 Traversal 查询只读缓存。
	// TODO: 如果可攀爬物数量很大，可以改成按需解析或空间索引。
	RefreshResolvedLedgePairs();
}

// 找到可供翻越的边缘数据并写入CheckResult
// TODO:细化失败原因
bool UGAFTraversableLedgeProviderComponent::GetLedgeTransforms(
	const FVector& HitLocation,
	const FVector& ActorLocation,
	FGAFTraversalCheckResult& InOutCheckResult) const
{
	InOutCheckResult.Reset();

	// Step 1 尝试找到离角色最近的 Ledge
	FGAFResolvedTraversableLedgeSelection Selection;
	if (!FindLedgeClosestToActor(ActorLocation, Selection))
	{
		InOutCheckResult.FailureReason = EGAFTraversalFailureReason::TraversalCheckFailed;
		InOutCheckResult.bHasFrontLedge = false;
		InOutCheckResult.bHasBackLedge = false;
		return false;
	}

	const UGAFTraversableLedgeSplineComponent* FrontLedge = Selection.FrontLedge.Get();
	// 至少需要找到一条边
	if (!IsValid(FrontLedge))
	{
		InOutCheckResult.FailureReason = EGAFTraversalFailureReason::TraversalCheckFailed;
		InOutCheckResult.bHasFrontLedge = false;
		InOutCheckResult.bHasBackLedge = false;
		return false;
	}

	// Step 2 保证 Ledge 长度足够
	if (FrontLedge->GetSplineLength() < MinLedgeWidth)
	{
		InOutCheckResult.FailureReason = EGAFTraversalFailureReason::TraversalCheckFailed;
		InOutCheckResult.bHasFrontLedge = false;
		InOutCheckResult.bHasBackLedge = false;
		return false;
	}

	// Step 3 对距离角色最近的点的位置进行限制，使它不能太靠近边缘的端点
	// 这里是让角色不能抓在 ledge 的最边缘位置，必须离边缘端点保留一段安全距离
	// 防止角色在靠近拐角处进行翻越时出现悬空
	// 限制后的边缘位置始终会距离边缘端点至少半个 “ 最小边缘宽度 ”
	// 如果最小边缘宽度是 60 单位，那么边缘位置就始终会距离拐角至少 30 单位
	const FVector ClosestFrontLedgeLocationLocal = FrontLedge->FindLocationClosestToWorldLocation(HitLocation, ESplineCoordinateSpace::Local);
	const float ClosestFrontLedgeDistance = FrontLedge->GetDistanceAlongSplineAtLocation(ClosestFrontLedgeLocationLocal, ESplineCoordinateSpace::Local);

	const float HalfMinLedgeWidth = MinLedgeWidth * 0.5f;
	// 限制距离
	const float ClampedFrontLedgeDistance = FMath::Clamp(ClosestFrontLedgeDistance, HalfMinLedgeWidth, FrontLedge->GetSplineLength() - HalfMinLedgeWidth);
	// 按限制后距离作为 Distance 重新取点
	const FTransform FrontLedgeTransform = FrontLedge->GetTransformAtDistanceAlongSpline(ClampedFrontLedgeDistance, ESplineCoordinateSpace::World, false);

	// 将 FrontLedge 写入结果
	InOutCheckResult.bHasFrontLedge = true;
	InOutCheckResult.FrontLedgeLocation = FrontLedgeTransform.GetLocation();
	InOutCheckResult.FrontLedgeNormal = FrontLedgeTransform.GetRotation().GetUpVector();

	const UGAFTraversableLedgeSplineComponent* BackLedge = Selection.BackLedge.Get();
	if (!IsValid(BackLedge))
	{
		// 只有单边 ledge 时允许返回 front ledge
		InOutCheckResult.bHasBackLedge = false;
		return true;
	}

	// Step 4 从 FrontLedgeLocation 出发，在配对的 BackLedge 上找最近的点作为 BackLedge 位置
	const FTransform BackLedgeTransform = BackLedge->FindTransformClosestToWorldLocation(
		InOutCheckResult.FrontLedgeLocation,
		ESplineCoordinateSpace::World,
		false);

	InOutCheckResult.bHasBackLedge = true;
	InOutCheckResult.BackLedgeLocation = BackLedgeTransform.GetLocation();
	InOutCheckResult.BackLedgeNormal = BackLedgeTransform.GetRotation().GetUpVector();

	return true;
}

// 遍历所有可用边缘，找到离角色最近的一条作为 FrontLedge，在同一组的就是 BackLedge
bool UGAFTraversableLedgeProviderComponent::FindLedgeClosestToActor(
	const FVector& ActorLocation,
	FGAFResolvedTraversableLedgeSelection& OutSelection) const
{
	// 选择结果缓存
	OutSelection = FGAFResolvedTraversableLedgeSelection{};

	if (ResolvedLedgePairs.IsEmpty())
	{
		UE_LOG(LogGAFTraversal, Warning, TEXT("Cannot find any enabled LedgePairs in [%s]."), *GetNameSafe(this));
		return false;
	}

	// 遍历所有 Spline 并对比距离，最小距离更新到 ClosestDistanceSq
	// 使用平方值，只比较大小不需要开根
	// 先用一个最大数初始化 ClosestDistance，确保第一次更新起效
	float ClosestDistanceSq{ TNumericLimits<float>::Max() };

	// 遍历每对Pair并尝试更新最佳结果
	for (const FGAFResolvedTraversableLedgePair& Pair : ResolvedLedgePairs)
	{
		TryUpdateClosestLedgeFromPair(Pair, ActorLocation, ClosestDistanceSq, OutSelection);
	}

	return IsValid(OutSelection.FrontLedge.Get());
}

// Pair内部对每条边进行对比，判断是否能作为最近的边缘
bool UGAFTraversableLedgeProviderComponent::TryUpdateClosestLedgeFromPair(
	const FGAFResolvedTraversableLedgePair& Pair,
	const FVector& ActorLocation,
	float& InOutClosestDistanceSq,
	FGAFResolvedTraversableLedgeSelection& OutSelection) const
{
	UGAFTraversableLedgeSplineComponent* FirstLedge = Pair.FirstLedge.Get();
	UGAFTraversableLedgeSplineComponent* SecondLedge = Pair.SecondLedge.Get();

	FGAFTraversablePairCandidate PairCandidate;

	// Pair 本身不固定 front/back，两条边都尝试更新后保留 Pair 内更近的结果
	// 同时传入两条边方便选出 Front 后对应出 Back
	TryUpdatePairCandidate(FirstLedge, SecondLedge, ActorLocation, PairCandidate);
	TryUpdatePairCandidate(SecondLedge, FirstLedge, ActorLocation, PairCandidate);

	// 如果这对 Pair 的最小距离大于已有候选 Pair 则不更新
	if (!PairCandidate.bHasCandidate || PairCandidate.DistanceSq >= InOutClosestDistanceSq)
	{
		return false;
	}

	// 当前 Pair 更佳，写入更新结果
	InOutClosestDistanceSq = PairCandidate.DistanceSq;
	OutSelection.FrontLedge = PairCandidate.FrontLedge;
	OutSelection.BackLedge = PairCandidate.BackLedge;
	OutSelection.FrontInputKey = PairCandidate.FrontInputKey;

	return true;
}

// 尝试将 Pair 内的一条 Spline 作为 FrontLedge 候选
// CandidateFront 是当前尝试面对角色的一侧，CandidateBack 是它配对的另一侧
// 如果 CandidateFront 比当前 PairCandidate 更近，则覆盖 PairCandidate
void UGAFTraversableLedgeProviderComponent::TryUpdatePairCandidate(
	UGAFTraversableLedgeSplineComponent* CandidateFront,
	UGAFTraversableLedgeSplineComponent* CandidateBack,
	const FVector& ActorLocation,
	FGAFTraversablePairCandidate& InOutPairCandidate) const
{
	if (!IsValid(CandidateFront) || !CandidateFront->bEnabled || CandidateFront->GetNumberOfSplinePoints() <= 0)
	{
		return;
	}

	// 找到角色位置投影到 CandidateFront 上的最近点
	const float FrontInputKey = CandidateFront->FindInputKeyClosestToWorldLocation(ActorLocation);
	const FVector FrontLocation = CandidateFront->GetLocationAtSplineInputKey(
		FrontInputKey,
		ESplineCoordinateSpace::World);

	// 暂时与 GASP 的 FindLedgeClosestToActor 逻辑保持一致
	// 用朝向角色侧(可攀爬侧)的法线给最近点一个小偏移，减少拐角或端点重合时的选择抖动
	// 比如两个 Spline 在端点重合，从对着端点测开始检测就会出现距离一样的情况
	// 这时候把端点往朝角色所在侧轻轻推 10cm，再比较距离，更偏向角色的点距离会更小
	// 先抽离为单独函数，后面尝试拓展逻辑
	const FVector FrontNormal = GetLedgeNormalFacingLocation(
		*CandidateFront,
		FrontInputKey,
		ActorLocation);
	constexpr float SelectionNormalOffset{ 10.0f };
	// 把点向法线方向偏移一段距离
	const FVector AdjustedFrontLocation = FrontLocation + FrontNormal * SelectionNormalOffset;
	// 取偏移后的距离
	const float DistanceSq = FVector::DistSquared2D(ActorLocation, AdjustedFrontLocation);

	// 如果已有候选 Front 且当前距离比候选者大就不进行更新
	if (InOutPairCandidate.bHasCandidate && DistanceSq >= InOutPairCandidate.DistanceSq)
	{
		return;
	}

	// 当前候选是这个 pair 内目前更近的一侧，更新临时结果
	InOutPairCandidate.bHasCandidate = true;
	InOutPairCandidate.FrontLedge = CandidateFront;
	// Back 就是 Pair 内对应的另一边
	InOutPairCandidate.BackLedge = CandidateBack;
	InOutPairCandidate.FrontInputKey = FrontInputKey;
	InOutPairCandidate.DistanceSq = DistanceSq;
}

// 返回 InputKey 在 Ledge 上偏向 TargetLocation 侧的法线
FVector UGAFTraversableLedgeProviderComponent::GetLedgeNormalFacingLocation(
	const UGAFTraversableLedgeSplineComponent& Ledge,
	const float InputKey,
	const FVector& TargetLocation)
{
	// 默认把 Spline 在该点的 UpVector 当作 ledge 法线
	// 这要求编辑 Spline 时保持局部上方向(Z)指向边缘的可攀爬侧
	// 不过 SplineComponent 也可以在 Detail 设置 Default Up Vector，不一定强制要求 Z
	// TODO: 尝试放宽Spline编辑要求
	FVector Normal = Ledge.GetUpVectorAtSplineInputKey(InputKey, ESplineCoordinateSpace::World).GetSafeNormal();

	(void)TargetLocation;
	// // 这里可以做根据点到角色方向的自动翻转机制，但是会破坏 “ 边缘只有一侧可攀爬 ” 的语义，需要进一步更改逻辑
	// const FVector LedgeLocation = Ledge.GetLocationAtSplineInputKey(InputKey, ESplineCoordinateSpace::World);
	//
	// // Ledge 点到目标点的方向
	// // 只比较水平面方向，避免角色和 Ledge 的高度差影响法线朝向判断
	// FVector DirectionToTarget = TargetLocation - LedgeLocation;
	// DirectionToTarget.Z = 0.0f;
	//
	// // 如果默认法线背对目标点，就翻转它，保证返回值始终朝向 TargetLocation 所在侧
	// DirectionToTarget.Normalize();
	// // 使用点积判断
	// if (FVector::DotProduct(Normal, DirectionToTarget) < 0.0f)
	// {
	// 	Normal *= -1.0f;
	// }

	return Normal;
}

// 将编辑器里的 LedgePairs 解析为运行时组件指针
void UGAFTraversableLedgeProviderComponent::RefreshResolvedLedgePairs()
{
	ResolvedLedgePairs.Reset();

	for (const FGAFTraversableLedgePair& Pair : LedgePairs)
	{
		if (!Pair.bEnabled)
		{
			continue;
		}

		UGAFTraversableLedgeSplineComponent* FirstLedge = ResolveLedgeSpline(Pair.FirstLedge);
		UGAFTraversableLedgeSplineComponent* SecondLedge = ResolveLedgeSpline(Pair.SecondLedge);

		// 两条边都无效则不能作为 Pair，但是允许单边无效
		if (!IsValid(FirstLedge) && !IsValid(SecondLedge))
		{
			UE_LOG(LogGAFTraversal, Warning, TEXT("%s skipped a LedgePair: both ledges are invalid."), *GetNameSafe(this));
			continue;
		}

		FGAFResolvedTraversableLedgePair ResolvedPair;
		ResolvedPair.FirstLedge = FirstLedge;
		ResolvedPair.SecondLedge = SecondLedge;

		ResolvedLedgePairs.Add(ResolvedPair);
	}
}

// 获取所有的LedgeSpline组件
// TODO: 可用于Debug绘制所有组件
void UGAFTraversableLedgeProviderComponent::GetLedgeSplines(TArray<UGAFTraversableLedgeSplineComponent*>& OutSplines) const
{
	OutSplines.Reset();

	const AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		UE_LOG(LogGAFTraversal, Warning, TEXT("Cannot get LedgeSplineComponents on [%s]: Owner is invalid."), *GetNameSafe(this));
		return;
	}

	Owner->GetComponents<UGAFTraversableLedgeSplineComponent>(OutSplines);
}

// 解析单个SplineComponent
UGAFTraversableLedgeSplineComponent* UGAFTraversableLedgeProviderComponent::ResolveLedgeSpline(
	const FComponentReference& Reference) const
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		UE_LOG(LogGAFTraversal, Warning, TEXT("Cannot resolve LedgeSplineComponent on [%s]: Owner is invalid."), *GetNameSafe(this));
		return nullptr;
	}

	// 将组件引用解析为组件指针
	return Cast<UGAFTraversableLedgeSplineComponent>(Reference.GetComponent(Owner));
}
