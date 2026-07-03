#include "AnimInstance/GAFAnimInstance.h"

#include "GAFLogChannels.h"
#include "PoseSearch/AnimNode_PoseSearchHistoryCollector.h"
#include "PoseSearch/PoseSearchLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFAnimInstance)

void UGAFAnimationInstance::SetTraversalInteractionTransform(const FTransform& InInteractionTransform)
{
	TraversalInteractionTransform = InInteractionTransform;
}

// 传入 Tag 找到 PoseHistory 节点并获取 PoseHistory
bool UGAFAnimationInstance::GetTraversalPoseHistoryReference(
	const FName PoseHistoryTag,
	FPoseHistoryReference& OutPoseHistory) const
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
