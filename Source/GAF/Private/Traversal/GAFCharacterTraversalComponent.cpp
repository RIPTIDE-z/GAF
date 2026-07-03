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

	const FVector ActorLocation = Character->GetActorLocation();

	// Step 2.1 Traversable Object Search: 往角色前方向进行一次Trace，尝试找到含有 TraversableLedgeProvider 的物体
	// 如果找到了，就设置 Hit Component, 反之退出并返回失败原因
	
	// 2.1.1 起点 = 角色位置 + 起点偏移值
	const FVector TraceStart = ActorLocation + Inputs.TraceOriginOffset;
	
	// 2.1.2 终点 = 起点 + 角色前向*Trace距离 + 终点偏移值
	const FVector TraceEnd =
		TraceStart
		+ Inputs.TraceForwardDirection * Inputs.TraceForwardDistance
		+ Inputs.TraceEndOffset;

	// 忽略自身
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Character);

	FHitResult TraversableObjectHitResult;
	const EDrawDebugTrace::Type EffectiveDebugType =
		TraversalSettings.DebugDrawLevel >= 2 ? DebugType : EDrawDebugTrace::None;

	// 2.1.3 使用 Capsule Trace
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
		TraversableObjectHitResult,
		true,
		FLinearColor::Black,
		FLinearColor::Black,
		TraversalSettings.DebugDrawDuration);

	if (!bHit)
	{
		OutResult.FailureReason = EGAFTraversalFailureReason::CantFindTraversableObject;
		return false;
	}

	const AActor* HitActor = TraversableObjectHitResult.GetActor();
	if (!IsValid(HitActor))
	{
		OutResult.FailureReason = EGAFTraversalFailureReason::CantFindTraversableObject;
		return false;
	}

	TArray<UGAFTraversableLedgeProviderComponent*> TraversableProviders;
	HitActor->GetComponents<UGAFTraversableLedgeProviderComponent>(TraversableProviders);

	if (TraversableProviders.IsEmpty())
	{
		OutResult.FailureReason = EGAFTraversalFailureReason::CantFindTraversableObject;
		return false;
	}

	if (TraversableProviders.Num() > 1)
	{
		OutResult.FailureReason = EGAFTraversalFailureReason::CantFindTraversableObject;
		UE_LOG(LogGAFTraversal, Warning,
			TEXT("%s failed to try traversal action: hit actor [%s] has multiple TraversableLedgeProvider components. Only one provider is allowed per traversable actor."),
			*GetNameSafe(this),
			*GetNameSafe(HitActor));
		return false;
	}

	const UGAFTraversableLedgeProviderComponent* TraversableProvider = TraversableProviders[0];
	if (!IsValid(TraversableProvider))
	{
		OutResult.FailureReason = EGAFTraversalFailureReason::CantFindTraversableObject;
		return false;
	}

	OutResult.HitComponent = TraversableObjectHitResult.GetComponent();

	// Step 2.2 Get Ledge : 如果检测到了可翻越物体，调用其内部函数找到Front/Back Ledge
	FGAFTraversalCheckResult InOutTraversalCheckResult = FGAFTraversalCheckResult{};
	bool bGetLedge = TraversableProvider->GetLedgeTransforms(TraversableObjectHitResult.ImpactPoint, ActorLocation, InOutTraversalCheckResult);
	OutResult = InOutTraversalCheckResult;
	OutResult.HitComponent = TraversableObjectHitResult.GetComponent();
	
	// DEBUG(Step2): 绘制前后边缘位置
	// TODO:提取为单独模块
	if (TraversalSettings.DebugDrawLevel >= 1)
	{
		UWorld* World = GetWorld();
		if (IsValid(World))
		{
			if (OutResult.bHasFrontLedge)
			{
				DrawDebugSphere(
					World,
					OutResult.FrontLedgeLocation,
					10.0f,
					12,
					FColor::Green,
					false,
					TraversalSettings.DebugDrawDuration,
					0,
					1.0f);
			}

			if (OutResult.bHasBackLedge)
			{
				DrawDebugSphere(
					World,
					OutResult.BackLedgeLocation,
					10.0f,
					12,
					FColor::Blue,
					false,
					TraversalSettings.DebugDrawDuration,
					0,
					1.0f);
			}
		}
	}

	// Step 3.1 If the traversable level block has a valid front ledge, continue the function. If not, exit early.
	if (!OutResult.bHasFrontLedge)
	{
		OutResult.FailureReason = EGAFTraversalFailureReason::CantFindFrontLedge;
		return false;
	}

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
