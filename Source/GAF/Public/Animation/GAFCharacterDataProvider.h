#pragma once

#include "GAFAnimationTypes.h"
#include "GAFCharacterDataProvider.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UGAFCharacterDataProvider : public UInterface
{
	GENERATED_BODY()
};

class GAF_API IGAFCharacterDataProvider
{
	GENERATED_BODY()

public:
	// 动画蓝图数据获取接口
	virtual bool GetAnimationFrameData(FGAFAnimationFrameData& OutData) const = 0;
	// 摄像机参数获取接口
	virtual bool GetCameraFrameData(FGAFCameraFrameData& OutData) const = 0;
	// 翻越系统数据获取接口
	virtual bool GetTraversalFrameData(FGAFTraversalFrameData& OutData) const = 0;
};

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UGAFMovementDataProvider : public UInterface
{
	GENERATED_BODY()
};

class GAF_API IGAFMovementDataProvider
{
	GENERATED_BODY()

public:
	// CMC数据获取接口
	virtual bool GetMovementData(FGAFMovementData& OutData) const = 0;
};