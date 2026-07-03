#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

#include "GAFTraversalSettings.generated.h"

USTRUCT(BlueprintType)
struct GAF_API FGAFTraversalSettings
{
	GENERATED_BODY()

	// Traversal 前方检测胶囊半径
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Traversal|Trace", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float TraceCapsuleRadius{ 30.0f };

	// 地面状态检测胶囊半高
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Traversal|GroundTrace", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float GroundTraceCapsuleHalfHeight{ 60.0f };

	// 地面低速时的最小前方检测距离
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Traversal|GroundTrace", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float GroundMinTraceForwardDistance{ 75.0f };

	// 地面高速时的最大前方检测距离
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Traversal|GroundTrace", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float GroundMaxTraceForwardDistance{ 350.0f };

	// 地面前向速度达到该值时使用最大检测距离
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Traversal|GroundTrace", meta = (ClampMin = "1.0", UIMin = "1.0"))
	float GroundMaxTraceForwardSpeed{ 500.0f };

	// 空中状态检测胶囊半高
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Traversal|AirTrace", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float AirTraceCapsuleHalfHeight{ 86.0f };

	// 空中状态固定前方检测距离
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Traversal|AirTrace", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float AirTraceForwardDistance{ 75.0f };

	// 空中状态终点偏移
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Traversal|AirTrace")
	FVector AirTraceEndOffset{ 0.0f, 0.0f, 50.0f };

	// BackFloor 向下检测的额外距离，避免另一侧地面略低时漏检
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Traversal|Trace", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float BackFloorTraceExtraDistance{ 50.0f };

	// FrontLedge MotionWarping 目标名，需要与 Montage 中的 WarpTargetName 一致
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Traversal|MotionWarping")
	FName FrontLedgeWarpTargetName{ TEXT("FrontLedge") };

	// BackLedge MotionWarping 目标名，需要与 Montage 中的 WarpTargetName 一致
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Traversal|MotionWarping")
	FName BackLedgeWarpTargetName{ TEXT("BackLedge") };

	// BackFloor MotionWarping 目标名，需要与 Montage 中的 WarpTargetName 一致
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Traversal|MotionWarping")
	FName BackFloorWarpTargetName{ TEXT("BackFloor") };

	// Traversal 动画中记录离 FrontLedge 距离的曲线名
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Traversal|MotionWarping")
	FName DistanceFromLedgeCurveName{ TEXT("Distance_From_Ledge") };

	// 是否用角色配置覆盖 Project Settings 中的 Traversal Trace Channel
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Traversal|Collision")
	bool bOverrideTraversalTraceChannel{ false };

	// 角色专用 Traversal Trace Channel，仅在开启 Override 时生效
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Traversal|Collision", meta = (EditCondition = "bOverrideTraversalTraceChannel"))
	TEnumAsByte<ECollisionChannel> TraversalTraceChannel{ ECC_Visibility };

	// Trace Debug 绘制等级。0/1 不绘制，2 及以上按传入 DebugType 绘制
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Traversal|Debug", meta = (ClampMin = "0", ClampMax = "4", UIMin = "0", UIMax = "4"))
	int32 DebugDrawLevel{ 1 };

	// Trace Debug 保留时间，仅 ForDuration 等持续绘制模式使用
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Traversal|Debug", meta = (ClampMin = "0.0", ClampMax = "60", UIMin = "0.0"))
	float DebugDrawDuration{ 1.5f };

	// Traversal 屏幕调试文本颜色，用于 PrintString 输出失败原因等状态
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Traversal|Debug")
	FLinearColor DebugPrintColor{ 0.0f, 0.8f, 1.0f, 1.0f };

	// Traversal 屏幕调试文本显示时间，用于 PrintString
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Traversal|Debug", meta = (ClampMin = "0.0", ClampMax = "60.0", UIMin = "0.0"))
	float DebugPrintDuration{ 2.0f };
};
