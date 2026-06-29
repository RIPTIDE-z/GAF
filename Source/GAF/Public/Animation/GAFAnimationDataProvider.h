#pragma once

#include "GAFAnimationTypes.h"
#include "GAFAnimationDataProvider.generated.h"

UINTERFACE(MinimalAPI)
class UGAFAnimationDataProvider : public UInterface
{
	GENERATED_BODY()
};

class GAF_API IGAFAnimationDataProvider
{
	GENERATED_BODY()

public:
	virtual bool GetAnimationData(FGAFAnimationFrameData& OutData) const = 0;
};