#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Component/GAFTraversalComponent.h"
#include "Engine/EngineTypes.h"

#include "GAFTraversableComponent.generated.h"

class UGAFTraversableLedgeSplineComponent;

// 两两Ledge配对的Pair
USTRUCT(BlueprintType)
struct GAF_API FGAFTraversableLedgePair
{
	GENERATED_BODY()

	// 是否启用该组 Front / Back 边缘配对。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAF|Traversal")
	bool bEnabled{ true };

	// 靠近角色的一侧边缘
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAF|Traversal",
		meta = (UseComponentPicker, AllowedClasses = "/Script/GAF.GAFTraversableLedgeSplineComponent", AllowAnyActor = "false"))
	FComponentReference FrontLedge;

	// 障碍物背侧边缘，可为空。为空时只提供 FrontLedge
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAF|Traversal",
		meta = (UseComponentPicker, AllowedClasses = "/Script/GAF.GAFTraversableLedgeSplineComponent", AllowAnyActor = "false"))
	FComponentReference BackLedge;
};

UCLASS(ClassGroup = (GAF), meta = (BlueprintSpawnableComponent))
class GAF_API UGAFTraversableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGAFTraversableComponent();

	// 根据命中位置和角色位置获取可攀爬边缘数据
	// 传入一个TraversalCheckResult并对其进行直接修改
	UFUNCTION(BlueprintCallable, Category = "GAF|Traversal")
	bool GetLedgeTransforms(
		const FVector& HitLocation,
		const FVector& ActorLocation,
		FGAFTraversalCheckResult& InOutCheckResult) const;

protected:
	void GetLedgeSplines(TArray<UGAFTraversableLedgeSplineComponent*>& OutSplines) const;
	UGAFTraversableLedgeSplineComponent* ResolveLedgeSpline(const FComponentReference& Reference) const;

protected:
	// 显式配置 Front / Back 边缘对应关系
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAF|Traversal")
	TArray<FGAFTraversableLedgePair> LedgePairs;
};
