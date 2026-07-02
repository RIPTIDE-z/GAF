#include "Component/GAFTraversalComponent.h"

#include "GAFLogChannels.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFTraversalComponent)

UGAFTraversalComponent::UGAFTraversalComponent()
{
	// 翻越组件不需要Tick，每次翻越才会调用，状态也都是临时状态
	PrimaryComponentTick.bCanEverTick = false;
}

bool UGAFTraversalComponent::TryTraversalAction(
	const FGAFTraversalCheckInputs& Inputs,
	const EGAFTraversalDebugType DebugType,
	FGAFTraversalCheckResult& OutResult)
{
	(void)Inputs;
	(void)DebugType;

	OutResult = FGAFTraversalCheckResult{};

	const ACharacter* Character = GetOwnerCharacter();
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

	// TODO: Traversal 检测、动作类型选择和 Montage 播放逻辑。
	OutResult.FailureReason = EGAFTraversalFailureReason::TraversalCheckFailed;
	return false;
}

ACharacter* UGAFTraversalComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}
