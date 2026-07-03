#pragma once

#include "Animation/AnimInstance.h"
#include "AnimInstance/GAFAnimInstanceDataProvider.h"
#include "PoseSearch/PoseSearchHistory.h"

#include "GAFAnimInstance.generated.h"

class UGAFAnimInstanceSettings;

UCLASS()
class GAF_API UGAFAnimationInstance :
	public UAnimInstance,
	public IGAFAnimInstanceDataProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "GAF|Traversal")
	virtual void SetTraversalInteractionTransform(const FTransform& InInteractionTransform) override;

	const FTransform& GetTraversalInteractionTransform() const { return TraversalInteractionTransform; }

	virtual bool GetTraversalPoseHistoryReference(FName PoseHistoryTag, FPoseHistoryReference& OutPoseHistory) const override;

protected:
	// Traversal Motion Matching 使用的交互目标，由 CharacterTraversalComponent 在检测成功后写入
	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Traversal")
	FTransform TraversalInteractionTransform{ FTransform::Identity };
};
