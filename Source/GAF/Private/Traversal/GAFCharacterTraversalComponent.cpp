#include "Traversal/GAFCharacterTraversalComponent.h"

#include "GAFLogChannels.h"
#include "Traversal/GAFTraversalCollisionResolver.h"
#include "Traversal/GAFTraversableLedgeProviderComponent.h"
#include "Animation/AnimationTypes.h"
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
	const FGAFTraversalCheckInputs& Inputs,
	const FGAFTraversalSettings& TraversalSettings,
	const EDrawDebugTrace::Type DebugType,
	FGAFTraversalCheckResult& OutResult)
{
	// 存放检测结果
	OutResult.Reset();

	ACharacter* Character = GetOwnerCharacter();
	if (!IsValid(Character))
	{
		OutResult.FailureReason = EGAFTraversalFailureReason::InvalidOwner;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to try traversal action: owner is not a valid Character."), *GetNameSafe(this));
		return false;
	}

	if (!IsValid(Character->GetCharacterMovement()))
	{
		OutResult.FailureReason = EGAFTraversalFailureReason::InvalidMovementComponent;
		UE_LOG(LogGAFTraversal, Warning, TEXT("%s failed to try traversal action: CMC is invalid on [%s]."), *GetNameSafe(this), *GetNameSafe(Character));
		return false;
	}

	if (bDoingTraversalAction)
	{
		OutResult.FailureReason = EGAFTraversalFailureReason::AlreadyDoingTraversal;
		return false;
	}

	// 优先使用角色配置覆盖的 TraceChannel，否则回退到项目级 TraversalConfig。
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

	const FVector ActorLocation = Character->GetActorLocation();

	// Step 2.1 : 往角色前方向进行一次Trace，尝试找到含有 TraversableLedgeProvider 的物体
	// 如果找到了，就设置 Hit Component, 反之退出并返回失败原因
	const FVector TraceStart = ActorLocation + Inputs.TraceOriginOffset;
	const FVector TraceEnd =
		TraceStart
		+ Inputs.TraceForwardDirection * Inputs.TraceForwardDistance
		+ Inputs.TraceEndOffset;

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Character);

	FHitResult Hit;
	const EDrawDebugTrace::Type EffectiveDebugType =
		TraversalSettings.DebugDrawLevel >= 2 ? DebugType : EDrawDebugTrace::None;

	const bool bHit = UKismetSystemLibrary::CapsuleTraceSingle(
		this,
		TraceStart,
		TraceEnd,
		Inputs.TraceRadius,
		Inputs.TraceHalfHeight,
		UEngineTypes::ConvertToTraceType(TraversalTraceChannel),
		false,
		ActorsToIgnore,
		EffectiveDebugType,
		Hit,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		TraversalSettings.DebugDrawDuration);

	if (!bHit)
	{
		OutResult.FailureReason = EGAFTraversalFailureReason::CantFindTraversableObject;
		return false;
	}

	AActor* HitActor = Hit.GetActor();
	const UGAFTraversableLedgeProviderComponent* TraversableProvider =
		IsValid(HitActor)
		? HitActor->FindComponentByClass<UGAFTraversableLedgeProviderComponent>()
		: nullptr;

	if (!IsValid(TraversableProvider))
	{
		OutResult.FailureReason = EGAFTraversalFailureReason::CantFindTraversableObject;
		return false;
	}

	OutResult.HitComponent = Hit.GetComponent();

	// Step 2.2 : If a traversable level block was found, get the front and back ledge transforms from it (using its own internal function).

	// DEBUG: Draw Debug shapes at ledge locations.

	// Step 3.1 If the traversable level block has a valid front ledge, continue the function. If not, exit early.

	// Step 3.2: Perform a trace from the actors location up to the front ledge location to determine
	// if there is room for the actor to move up to it. If so, continue the function. If not, exit early.

	// Step 3.3: save the height of the obstacle using the delta between the actor and front ledge transform.

	// Step 3.4: Perform a trace across the top of the obstacle from the front ledge to the back ledge
	// to see if there's room for the actor to move across it.

	// Step 3.5: If there is room, save the obstacle depth using the difference between the front and back ledge locations.

	// Step 3.6: Trace downward from the back ledge location (using the height of the obstacle for the distance) to find the floor.
	// If there is a floor, save its location and the back ledges height (using the distance between the back ledge and the floor).
	// If no floor was found, invalidate the back floor.

	// Step 3.5: If there is not room, save the obstacle depth using the difference between the
	// front ledge and the trace impact point, and invalidate the back ledge.

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

ACharacter* UGAFCharacterTraversalComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}
