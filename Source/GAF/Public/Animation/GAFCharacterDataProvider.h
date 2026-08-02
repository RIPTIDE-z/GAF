#pragma once

#include "GAFAnimationTypes.h"
#include "GAFCharacterDataProvider.generated.h"

// 角色数据提供者接口
// BlueprintType 允许蓝图识别该接口类型
UINTERFACE(BlueprintType)
class UGAFCharacterDataProvider : public UInterface
{
	GENERATED_BODY()
};

class GAF_API IGAFCharacterDataProvider
{
	GENERATED_BODY()

public:
	// 获取当前帧动画所需数据
	// BlueprintNativeEvent 允许蓝图子类覆写；BlueprintCallable 允许蓝图侧调用
	// C++ 默认实现名为 GetAnimationFrameData_Implementation
	// 注意 BlueprintNativeEvent 不能是 const 成员函数
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GAF|Animation")
	bool GetAnimationFrameData(FGAFAnimationFrameData& OutData);

	// 获取当前帧相机参数
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GAF|Camera")
	bool GetCameraFrameData(FGAFCameraFrameData& OutData);

	// 获取当前帧翻越系统所需角色数据
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GAF|Traversal")
	bool GetTraversalFrameData(FGAFTraversalFrameData& OutData);
};

// 移动数据提供者接口
// CMC 通过该接口读取每帧移动参数，不直接依赖具体 Character 子类
UINTERFACE(BlueprintType)
class UGAFLocomotionDataProvider : public UInterface
{
	GENERATED_BODY()
};

class GAF_API IGAFLocomotionDataProvider
{
	GENERATED_BODY()

public:
	// 获取当前帧移动所需的速度、加速度、旋转模式等参数
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GAF|Movement")
	bool GetLocomotionData(FGAFLocomotionData& OutData);
};
