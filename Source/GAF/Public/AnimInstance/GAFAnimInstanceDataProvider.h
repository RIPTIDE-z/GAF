#pragma once

#include "UObject/Interface.h"
#include "PoseSearch/PoseSearchHistory.h"

#include "GAFAnimInstanceDataProvider.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UGAFAnimInstanceDataProvider : public UInterface
{
	GENERATED_BODY()
};

class GAF_API IGAFAnimInstanceDataProvider
{
	GENERATED_BODY()

public:
	virtual void SetTraversalInteractionTransform(const FTransform& InInteractionTransform) = 0;

	virtual bool GetTraversalPoseHistoryReference(FName PoseHistoryTag, FPoseHistoryReference& OutPoseHistory) const = 0;
};
