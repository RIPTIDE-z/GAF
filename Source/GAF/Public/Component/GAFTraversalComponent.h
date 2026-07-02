#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/GAFCharacterDataProvider.h"

#include "GAFTraversalComponent.generated.h"

class ACharacter;
class UAnimMontage;
class UPrimitiveComponent;

UENUM(BlueprintType)
enum class EGAFTraversalActionType : uint8
{
	None,
	// Traverse over a thin object and end on the ground at a similar level (Low fence)
	Hurdle,
	// Traverse over a thin object and end in a falling state (Tall fence, or elevated obstacle with no floor on the other side)
	Vault,
	// Traverse up and onto an object without passing over it
	Mantle
};

UENUM(BlueprintType)
enum class EGAFTraversalDebugType : uint8
{
	None,
	ForOneFrame,
	ForDuration
};

UENUM(BlueprintType)
enum class EGAFTraversalFailureReason : uint8
{
	None,
	TraversalCheckFailed,
	MontageSelectionFailed,
	AlreadyDoingTraversal,
	InvalidOwner,
	InvalidMovementComponent
};

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
	float TraceRadius{ 0.0f };

	// 用于Trace的胶囊体的半高度
	UPROPERTY(BlueprintReadOnly)
	float TraceHalfHeight{ 0.0f };
};

USTRUCT(BlueprintType)
struct GAF_API FGAFTraversalCheckResult
{
	GENERATED_BODY()

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

UCLASS(ClassGroup = (GAF), meta = (BlueprintSpawnableComponent))
class GAF_API UGAFTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGAFTraversalComponent();

	bool IsDoingTraversalAction() const { return bDoingTraversalAction; }

	// 检测是否能进行翻越并进行动画选择
	// 若检测判定失败则会将原因写入CheckResult
	UFUNCTION(BlueprintCallable, Category = "GAF|Traversal")
	bool TryTraversalAction(
		const FGAFTraversalCheckInputs& Inputs,
		EGAFTraversalDebugType DebugType,
		FGAFTraversalCheckResult& OutResult);

	// Debug绘制等级
	// TODO: 提升为控制台变量
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GAF|Traversal")
	int DebugDrawLevel{ 1 };

	// Debug绘制时间
	// TODO: 提升为控制台变量
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GAF|Traversal")
	float DebugDrawDuration{ 1.5f };

protected:
	ACharacter* GetOwnerCharacter() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Traversal")
	bool bDoingTraversalAction{ false };

	// 缓存的运动数据
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	FGAFTraversalFrameData CachedTraversalData;
};
