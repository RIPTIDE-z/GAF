#pragma once

#include "UObject/Interface.h"
#include "PoseSearch/PoseSearchHistory.h"

#include "GAFAnimInstanceDataProvider.generated.h"

// AnimInstance 数据提供者接口，负责写入 Traversal 交互目标和获取 PoseHistory
UINTERFACE(BlueprintType)
class UGAFAnimInstanceDataProvider : public UInterface
{
	GENERATED_BODY()
};

class GAF_API IGAFAnimInstanceDataProvider
{
	GENERATED_BODY()

public:
	// 写入 Traversal 交互目标 Transform，Motion Matching 自定义 Channel 使用
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GAF|Traversal")
	void SetTraversalInteractionTransform(const FTransform& InInteractionTransform);

	// 通过 AnimGraph 节点 Tag 查找 PoseHistory 引用，供 Chooser 评估使用
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GAF|Traversal")
	bool GetTraversalPoseHistoryReference(FName PoseHistoryTag, FPoseHistoryReference& OutPoseHistory);
};
