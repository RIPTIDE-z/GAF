#include "Component/GAFTraversableComponent.h"

#include "Component/GAFTraversableLedgeSplineComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFTraversableComponent)

UGAFTraversableComponent::UGAFTraversableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UGAFTraversableComponent::GetLedgeTransforms(
	const FVector& HitLocation,
	const FVector& ActorLocation,
	FGAFTraversalCheckResult& InOutCheckResult) const
{
	(void)HitLocation;
	(void)ActorLocation;
	(void)InOutCheckResult;

	TArray<UGAFTraversableLedgeSplineComponent*> LedgeSplines;
	GetLedgeSplines(LedgeSplines);

	// TODO: 根据 LedgePairs、LedgeSplines、命中位置和角色位置计算前/后边缘，并写入 InOutCheckResult。
	for (const FGAFTraversableLedgePair& Pair : LedgePairs)
	{
		if (!Pair.bEnabled)
		{
			continue;
		}

		UGAFTraversableLedgeSplineComponent* FrontLedge = ResolveLedgeSpline(Pair.FrontLedge);
		UGAFTraversableLedgeSplineComponent* BackLedge = ResolveLedgeSpline(Pair.BackLedge);
		(void)FrontLedge;
		(void)BackLedge;
	}

	return false;
}

void UGAFTraversableComponent::GetLedgeSplines(TArray<UGAFTraversableLedgeSplineComponent*>& OutSplines) const
{
	OutSplines.Reset();

	const AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	Owner->GetComponents<UGAFTraversableLedgeSplineComponent>(OutSplines);
}

UGAFTraversableLedgeSplineComponent* UGAFTraversableComponent::ResolveLedgeSpline(
	const FComponentReference& Reference) const
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return nullptr;
	}

	return Cast<UGAFTraversableLedgeSplineComponent>(Reference.GetComponent(Owner));
}
