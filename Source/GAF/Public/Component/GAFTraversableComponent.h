/**
 * 将 GASP里的 LevelBlock_Traversable的可攀爬特性单独抽离为一个组件
 * 并将原本硬编码 Map来对应 Front/Back 的方法拓展为使用自定义 Pair 结构对应
 * 编辑器中使用 FComponentReference，运行时在 BeginPlay 统一解析为实际引用
 */
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Component/GAFTraversalComponent.h"
#include "Engine/EngineTypes.h"

#include "GAFTraversableComponent.generated.h"

class UGAFTraversableLedgeSplineComponent;

// 编辑器中配置的一组可攀爬边缘配对
// 两条边没有固定 Front / Back 语义，运行时会根据角色位置自动判断哪一条是 FrontLedge
// 允许只配置其中一条边，这种情况下只会返回 FrontLedge
USTRUCT(BlueprintType)
struct GAF_API FGAFTraversableLedgePair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAF|Traversal")
	bool bEnabled{ true };

	// 使用 FComponentReference 以在编辑器中选择同一个 Actor 上的 LedgeSpline 组件
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAF|Traversal", meta = (UseComponentPicker, AllowedClasses = "/Script/GAF.GAFTraversableLedgeSplineComponent", AllowAnyActor = "false"))
	FComponentReference FirstLedge;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAF|Traversal", meta = (UseComponentPicker, AllowedClasses = "/Script/GAF.GAFTraversableLedgeSplineComponent", AllowAnyActor = "false"))
	FComponentReference SecondLedge;
};

// BeginPlay 后缓存出来的组件指针
struct FGAFResolvedTraversableLedgePair
{
	TWeakObjectPtr<UGAFTraversableLedgeSplineComponent> FirstLedge;
	TWeakObjectPtr<UGAFTraversableLedgeSplineComponent> SecondLedge;
};

// 单次查询选中的 ledge 结果
// InputKey 用于表示 spline 上的精确位置
// 比如1.5表示在1,2两个控制点的一半位置
struct FGAFResolvedTraversableLedgeSelection
{
	TWeakObjectPtr<UGAFTraversableLedgeSplineComponent> FrontLedge;
	TWeakObjectPtr<UGAFTraversableLedgeSplineComponent> BackLedge;
	float FrontInputKey{ 0.0f };
	float BackInputKey{ 0.0f };
};

// Pair 内部临时候选结果，只在查找最近 ledge 时使用
struct FGAFTraversablePairCandidate
{
	bool bHasCandidate{ false };

	UGAFTraversableLedgeSplineComponent* FrontLedge{ nullptr };
	UGAFTraversableLedgeSplineComponent* BackLedge{ nullptr };

	float FrontInputKey{ 0.0f };
	float BackInputKey{ 0.0f };
	float DistanceSq{ 0.0f };
};

// 为任意Actor提供 “ 可翻越 ” 特性的组件
UCLASS(ClassGroup = (GAF), meta = (BlueprintSpawnableComponent))
class GAF_API UGAFTraversableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGAFTraversableComponent();

	virtual void BeginPlay() override;

	// 根据命中位置和角色位置获取可攀爬边缘数据，并直接写入 TraversalCheckResult
	UFUNCTION(BlueprintCallable, Category = "GAF|Traversal")
	bool GetLedgeTransforms(
		const FVector& HitLocation,
		const FVector& ActorLocation,
		FGAFTraversalCheckResult& InOutCheckResult) const;

protected:
	void RefreshResolvedLedgePairs();
	void GetLedgeSplines(TArray<UGAFTraversableLedgeSplineComponent*>& OutSplines) const;

	// 将编辑器里的组件引用转换为运行时可用的组件指针
	UGAFTraversableLedgeSplineComponent* ResolveLedgeSpline(const FComponentReference& Reference) const;

	// 遍历所有 ledge pair，选出离角色最近的一条作为 FrontLedge
	bool FindLedgeClosestToActor(
		const FVector& ActorLocation,
		FGAFResolvedTraversableLedgeSelection& OutSelection) const;

	// 在一个 pair 内部判断哪条边更适合作为 FrontLedge，并尝试更新当前最近结果
	bool TryUpdateClosestLedgeFromPair(
		const FGAFResolvedTraversableLedgePair& Pair,
		const FVector& ActorLocation,
		float& InOutClosestDistanceSq,
		FGAFResolvedTraversableLedgeSelection& OutSelection) const;

	// 测试一条具体候选边，只有在 pair 内部更近时才更新 PairCandidate
	void TryUpdatePairCandidate(
		UGAFTraversableLedgeSplineComponent* CandidateFront,
		UGAFTraversableLedgeSplineComponent* CandidateBack,
		const FVector& ActorLocation,
		FGAFTraversablePairCandidate& InOutPairCandidate) const;

	// 返回朝向指定位置的 ledge 法线，用于区分角色所在侧和障碍物另一侧
	static FVector GetLedgeNormalFacingLocation(
		const UGAFTraversableLedgeSplineComponent& Ledge,
		float InputKey,
		const FVector& TargetLocation);

	// 显式配置可互相对应的两条边缘
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAF|Traversal")
	TArray<FGAFTraversableLedgePair> LedgePairs;

	// 运行时缓存解析后的Pair
	TArray<FGAFResolvedTraversableLedgePair> ResolvedLedgePairs;
};
