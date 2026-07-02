#include "Component/GAFTraversableComponent.h"

#include "GAFLogChannels.h"
#include "Component/GAFTraversableLedgeSplineComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFTraversableComponent)

UGAFTraversableComponent::UGAFTraversableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGAFTraversableComponent::BeginPlay()
{
	Super::BeginPlay();

	// 在 BeginPlay 统一解析编辑器配置，后续 Traversal 查询只读缓存。
	// TODO: 如果可攀爬物数量很大，可以改成按需解析或空间索引。
	RefreshResolvedLedgePairs();
}

bool UGAFTraversableComponent::GetLedgeTransforms(
	const FVector& HitLocation,
	const FVector& ActorLocation,
	FGAFTraversalCheckResult& InOutCheckResult) const
{
	(void)HitLocation;

	FGAFResolvedTraversableLedgeSelection Selection;
	if (!FindLedgeClosestToActor(ActorLocation, Selection))
	{
		InOutCheckResult.bHasFrontLedge = false;
		InOutCheckResult.bHasBackLedge = false;
		return false;
	}

	const UGAFTraversableLedgeSplineComponent* FrontLedge = Selection.FrontLedge.Get();
	// 至少需要找到一条边
	if (!IsValid(FrontLedge))
	{
		InOutCheckResult.bHasFrontLedge = false;
		InOutCheckResult.bHasBackLedge = false;
		return false;
	}

	// 写入角色当前面对的边缘
	InOutCheckResult.bHasFrontLedge = true;
	InOutCheckResult.FrontLedgeLocation = FrontLedge->GetLocationAtSplineInputKey(
		Selection.FrontInputKey,
		ESplineCoordinateSpace::World);
	InOutCheckResult.FrontLedgeNormal = GetLedgeNormalFacingLocation(
		*FrontLedge,
		Selection.FrontInputKey,
		ActorLocation);

	const UGAFTraversableLedgeSplineComponent* BackLedge = Selection.BackLedge.Get();
	if (!IsValid(BackLedge))
	{
		// 只有单边 ledge 时允许返回 front ledge
		InOutCheckResult.bHasBackLedge = false;
		return true;
	}

	// BackLedge 使用与 FrontLedge 相同的归一化 spline 位置，保持前后边缘对应
	InOutCheckResult.bHasBackLedge = true;
	InOutCheckResult.BackLedgeLocation = BackLedge->GetLocationAtSplineInputKey(
		Selection.BackInputKey,
		ESplineCoordinateSpace::World);
	InOutCheckResult.BackLedgeNormal = GetLedgeNormalFacingLocation(
		*BackLedge,
		Selection.BackInputKey,
		// 使用 front -> back 的延长方向，让 back normal 朝向障碍物另一侧
		InOutCheckResult.BackLedgeLocation + InOutCheckResult.BackLedgeLocation - InOutCheckResult.FrontLedgeLocation);

	return true;
}

// 将编辑器里的 LedgePairs 解析为运行时组件指针
void UGAFTraversableComponent::RefreshResolvedLedgePairs()
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
		if (!IsValid(FirstLedge) && !IsValid(SecondLedge))
		{
			UE_LOG(LogGAFTraversal, Warning, TEXT("%s skipped a LedgePair: both ledges are invalid."), *GetNameSafe(this));
			continue;
		}

		FGAFResolvedTraversableLedgePair ResolvedPair;
		ResolvedPair.FirstLedge = FirstLedge;
		// 允许只配置一条边，查询时有效的一边会被当作 FrontLedge
		ResolvedPair.SecondLedge = SecondLedge;

		ResolvedLedgePairs.Add(ResolvedPair);
	}
}

// 获取所有的LedgeSpline组件
// TODO: 可用于Debug绘制所有组件
void UGAFTraversableComponent::GetLedgeSplines(TArray<UGAFTraversableLedgeSplineComponent*>& OutSplines) const
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
UGAFTraversableLedgeSplineComponent* UGAFTraversableComponent::ResolveLedgeSpline(
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

// 遍历所有可用边缘，找到离角色最近的一条作为 FrontLedge，在同一组的就是 BackLedge
bool UGAFTraversableComponent::FindLedgeClosestToActor(
	const FVector& ActorLocation,
	FGAFResolvedTraversableLedgeSelection& OutSelection) const
{
	// 缓存选择结果
	OutSelection = FGAFResolvedTraversableLedgeSelection{};

	if (ResolvedLedgePairs.IsEmpty())
	{
		UE_LOG(LogGAFTraversal, Warning, TEXT("Cannot find any enabled LedgePairs in [%s]."), *GetNameSafe(this));
		return false;
	}

	float ClosestDistanceSq{ TNumericLimits<float>::Max() };

	// 遍历每对Pair并尝试更新最佳结果
	for (const FGAFResolvedTraversableLedgePair& Pair : ResolvedLedgePairs)
	{
		TryUpdateClosestLedgeFromPair(Pair, ActorLocation, ClosestDistanceSq, OutSelection);
	}

	return IsValid(OutSelection.FrontLedge.Get());
}

// Pair内部对每条边进行对比，判断是否能作为最近的边缘
bool UGAFTraversableComponent::TryUpdateClosestLedgeFromPair(
	const FGAFResolvedTraversableLedgePair& Pair,
	const FVector& ActorLocation,
	float& InOutClosestDistanceSq,
	FGAFResolvedTraversableLedgeSelection& OutSelection) const
{
	UGAFTraversableLedgeSplineComponent* FirstLedge = Pair.FirstLedge.Get();
	UGAFTraversableLedgeSplineComponent* SecondLedge = Pair.SecondLedge.Get();

	FGAFTraversablePairCandidate PairCandidate;

	// Pair 本身不固定 front/back，两条边都尝试更新后保留 Pair 内更近的结果
	TryUpdatePairCandidate(FirstLedge, SecondLedge, ActorLocation, PairCandidate);
	TryUpdatePairCandidate(SecondLedge, FirstLedge, ActorLocation, PairCandidate);

	if (!PairCandidate.bHasCandidate || PairCandidate.DistanceSq >= InOutClosestDistanceSq)
	{
		UE_LOG(LogGAFTraversal, Warning, TEXT("Cannot find any enabled LedgePairs in [%s]."), *GetNameSafe(this));
		return false;
	}

	InOutClosestDistanceSq = PairCandidate.DistanceSq;
	OutSelection.FrontLedge = PairCandidate.FrontLedge;
	OutSelection.BackLedge = PairCandidate.BackLedge;
	OutSelection.FrontInputKey = PairCandidate.FrontInputKey;
	OutSelection.BackInputKey = PairCandidate.BackInputKey;

	return true;
}

// 尝试将 Pair 内的一条 Spline 作为 FrontLedge 候选
// CandidateFront 是当前尝试面对角色的一侧，CandidateBack 是它配对的另一侧
// 如果 CandidateFront 比当前 PairCandidate 更近，则覆盖 PairCandidate
void UGAFTraversableComponent::TryUpdatePairCandidate(
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

	// 按角色到 spline 最近点的 2D 距离选择 FrontLedge
	const float DistanceSq = FVector::DistSquared2D(ActorLocation, FrontLocation);
	if (InOutPairCandidate.bHasCandidate && DistanceSq >= InOutPairCandidate.DistanceSq)
	{
		return;
	}

	// 当前候选是这个 pair 内目前更近的一侧，更新临时结果
	InOutPairCandidate.bHasCandidate = true;
	InOutPairCandidate.FrontLedge = CandidateFront;
	InOutPairCandidate.BackLedge = CandidateBack;
	InOutPairCandidate.FrontInputKey = FrontInputKey;
	InOutPairCandidate.BackInputKey = 0.0f;
	InOutPairCandidate.DistanceSq = DistanceSq;

	if (IsValid(CandidateBack) && CandidateBack->bEnabled && CandidateBack->GetNumberOfSplinePoints() > 0)
	{
		// 用 FrontLedge 上的归一化距离映射到 BackLedge，保证成对边缘的前后位置对应
		// 例如 FrontLedge 上 30% 的位置，会对应到 BackLedge 上 30% 的位置
		const float FrontLength = FMath::Max(CandidateFront->GetSplineLength(), KINDA_SMALL_NUMBER);
		const float FrontDistance = CandidateFront->GetDistanceAlongSplineAtSplineInputKey(FrontInputKey);
		const float NormalizedDistance = FMath::Clamp(FrontDistance / FrontLength, 0.0f, 1.0f);
		InOutPairCandidate.BackInputKey = CandidateBack->GetInputKeyAtDistanceAlongSpline(
			NormalizedDistance * CandidateBack->GetSplineLength());
	}
}

FVector UGAFTraversableComponent::GetLedgeNormalFacingLocation(
	const UGAFTraversableLedgeSplineComponent& Ledge,
	const float InputKey,
	const FVector& TargetLocation)
{
	// 默认把 spline 在该点的 RightVector 当作 ledge 法线
	// 这要求编辑 ledge spline 时保持局部右方向大致垂直于边缘
	FVector Normal = Ledge.GetRightVectorAtSplineInputKey(InputKey, ESplineCoordinateSpace::World).GetSafeNormal();
	const FVector LedgeLocation = Ledge.GetLocationAtSplineInputKey(InputKey, ESplineCoordinateSpace::World);

	// 只比较水平面方向，避免角色和 ledge 的高度差影响法线朝向判断
	FVector DirectionToTarget = TargetLocation - LedgeLocation;
	DirectionToTarget.Z = 0.0f;

	if (DirectionToTarget.IsNearlyZero())
	{
		// 目标点几乎就在 ledge 点上时无法判断朝向，返回 spline 自身法线
		return Normal;
	}

	// 如果默认法线背对目标点，就翻转它，保证返回值始终朝向 TargetLocation 所在侧
	DirectionToTarget.Normalize();
	if (FVector::DotProduct(Normal, DirectionToTarget) < 0.0f)
	{
		Normal *= -1.0f;
	}

	return Normal;
}
