// Traversal 系统所使用的数据结构以及枚举
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PoseSearch/PoseSearchHistory.h"

#include "GAFTraversalTypes.generated.h"

class UAnimMontage;
class UPrimitiveComponent;

// 翻越动作类型
UENUM(BlueprintType)
enum class EGAFTraversalActionType : uint8
{
	None,
	// 越过一个较薄的物体，并在相近高度的地面上结束动作（例如低矮围栏）
	Hurdle,
	// 越过一个较薄的物体，并以空中下落状态结束动作（例如较高围栏，或另一侧没有地面的高处障碍物）
	Vault,
	// 向上攀爬到物体顶部，不越过该物体
	Mantle
};

// 翻越检测失败原因
UENUM(BlueprintType)
enum class EGAFTraversalFailureReason : uint8
{
	None,
	InvalidOwner,
	InvalidMovementComponent,
	AlreadyDoingTraversal,
	CantFindTraversableObject,
	CantFindFrontLedge,
	NoRoomMoveToFrontLedge,
	InvalidAnimInstance,
	InvalidPoseHistory,
	InvalidTraversalChooser,
	TraversalCheckFailed,
	MontageSelectionFailed,
};

// 翻越检测的输入，由 Character 输入给 CharacterTraversalComponent
USTRUCT(BlueprintType)
struct GAF_API FGAFTraversalCheckInputs
{
	GENERATED_BODY()

	// 障碍物检测的向前方向
	UPROPERTY(BlueprintReadOnly)
	FVector TraceForwardDirection{ ForceInit };

	// Trace距离
	UPROPERTY(BlueprintReadOnly)
	float TraceForwardDistance{ 0.0f };

	// 起点的偏移值
	UPROPERTY(BlueprintReadOnly)
	FVector TraceOriginOffset{ ForceInit };

	// 重点偏移值
	UPROPERTY(BlueprintReadOnly)
	FVector TraceEndOffset{ ForceInit };

	// 用于Trace的胶囊体的半径
	UPROPERTY(BlueprintReadOnly)
	float TraceCapsuleRadius{ 0.0f };

	// 用于Trace的胶囊体的半高度
	UPROPERTY(BlueprintReadOnly)
	float TraceCapsuleHalfHeight{ 0.0f };
};

// 翻越检测结果
USTRUCT(BlueprintType)
struct GAF_API FGAFTraversalCheckResult
{
	GENERATED_BODY()

	void Reset()
	{
		*this = FGAFTraversalCheckResult{};
	}

	// 翻越动作类型
	UPROPERTY(BlueprintReadOnly)
	EGAFTraversalActionType ActionType{ EGAFTraversalActionType::None };

	// 是否有前边缘
	UPROPERTY(BlueprintReadOnly)
	bool bHasFrontLedge{ false };

	// 前边缘位置
	UPROPERTY(BlueprintReadOnly)
	FVector FrontLedgeLocation{ ForceInit };

	// 前边缘法线方向
	UPROPERTY(BlueprintReadOnly)
	FVector FrontLedgeNormal{ ForceInit };

	// 是否有后边缘
	UPROPERTY(BlueprintReadOnly)
	bool bHasBackLedge{ false };

	// 后边缘位置
	UPROPERTY(BlueprintReadOnly)
	FVector BackLedgeLocation{ ForceInit };

	// 后边缘法线方向
	UPROPERTY(BlueprintReadOnly)
	FVector BackLedgeNormal{ ForceInit };

	// 后边缘后是否有可落地地面
	UPROPERTY(BlueprintReadOnly)
	bool bHasBackFloor{ false };

	// 可落地地面位置
	UPROPERTY(BlueprintReadOnly)
	FVector BackFloorLocation{ ForceInit };

	// 翻越物高度
	UPROPERTY(BlueprintReadOnly)
	float ObstacleHeight{ 0.0f };

	// 翻越物跨度(顶部长度)
	UPROPERTY(BlueprintReadOnly)
	float ObstacleDepth{ 0.0f };

	// 后边缘高度
	UPROPERTY(BlueprintReadOnly)
	float BackLedgeHeight{ 0.0f };

	// Trace碰撞到的组件
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPrimitiveComponent> HitComponent{ nullptr };

	// 选中的蒙太奇
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAnimMontage> ChosenMontage{ nullptr };

	UPROPERTY(BlueprintReadOnly)
	float StartTime{ 0.0f };

	UPROPERTY(BlueprintReadOnly)
	float PlayRate{ 1.0f };

	// Traversal 失败原因
	UPROPERTY(BlueprintReadOnly)
	EGAFTraversalFailureReason FailureReason{ EGAFTraversalFailureReason::None };
};

// Chooser 选择 Traversal Montage 所需的输入
USTRUCT(BlueprintType)
struct GAF_API FGAFTraversalChooserInputs
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EGAFTraversalActionType ActionType{ EGAFTraversalActionType::None };

	UPROPERTY(BlueprintReadWrite)
	bool bHasFrontLedge{ false };

	UPROPERTY(BlueprintReadWrite)
	bool bHasBackLedge{ false };

	UPROPERTY(BlueprintReadWrite)
	bool bHasBackFloor{ false };

	UPROPERTY(BlueprintReadWrite)
	float ObstacleHeight{ 0.0f };

	UPROPERTY(BlueprintReadWrite)
	float ObstacleDepth{ 0.0f };

	UPROPERTY(BlueprintReadWrite)
	float BackLedgeHeight{ 0.0f };

	UPROPERTY(BlueprintReadWrite)
	float DistanceToLedge{ 0.0f };

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag MovementMode;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag Gait;

	UPROPERTY(BlueprintReadWrite)
	float Speed{ 0.0f };

	UPROPERTY(BlueprintReadWrite)
	FPoseHistoryReference PoseHistory;
};

// Chooser 输出的 Traversal Montage 选择结果
USTRUCT(BlueprintType)
struct GAF_API FGAFTraversalChooserOutputs
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EGAFTraversalActionType ActionType{ EGAFTraversalActionType::None };

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimMontage> ChosenMontage{ nullptr };

	UPROPERTY(BlueprintReadWrite)
	float MontageStartTime{ 0.0f };
};
