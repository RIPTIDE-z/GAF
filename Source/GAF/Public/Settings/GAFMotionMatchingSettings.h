#pragma once

#include "CoreMinimal.h"

#include "GAFMotionMatchingSettings.generated.h"

class UChooserTable;

USTRUCT(BlueprintType)
struct GAF_API FGAFMotionMatchingSettings
{
	GENERATED_BODY()

	// Traversal 用于选择 Montage 的 Chooser 资产
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MotionMatching|Traversal")
	TObjectPtr<const UChooserTable> TraversalMontageChooser{ nullptr };

	// AnimGraph 中 Pose Search History Collector 节点的 Tag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MotionMatching|Traversal")
	FName TraversalPoseHistoryTag{ TEXT("PoseHistory") };

	// Traversal Montage 默认播放速率
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MotionMatching|Traversal", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DefaultTraversalPlayRate{ 1.0f };
};
