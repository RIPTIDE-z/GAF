#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/GAFCharacterDataProvider.h"
#include "Traversal/GAFTraversalTypes.h"

#include "GAFCharacterTraversalComponent.generated.h"

class ACharacter;

UCLASS(ClassGroup = (GAF), meta = (BlueprintSpawnableComponent))
class GAF_API UGAFCharacterTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGAFCharacterTraversalComponent();

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
