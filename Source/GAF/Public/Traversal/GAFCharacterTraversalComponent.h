#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/GAFCharacterDataProvider.h"
#include "Settings/GAFMotionMatchingSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Settings/GAFTraversalSettings.h"
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
		const FGAFTraversalCheckInputs& TraversalCheckInputs,
		const FGAFTraversalSettings& TraversalSettings,
		const FGAFMotionMatchingSettings& MotionMatchingSettings,
		EDrawDebugTrace::Type TraversalDebugType,
		FGAFTraversalCheckResult& InOutTraversalCheckResult);
	
	UFUNCTION(BlueprintCallable, Category = "GAF|Traversal")
	bool PerformTraversalAction(
		const FGAFTraversalCheckResult& TraversalResult,
		const FGAFTraversalSettings& TraversalSettings);
	
	UFUNCTION(BlueprintCallable, Category = "GAF|Traversal")
	void UpdateWarpTargets();

	// 打印 Traversal 检测失败原因
	void DebugPrintTraversalFailureReason(
		const FGAFTraversalCheckResult& TraversalCheckResult,
		const FGAFTraversalSettings& TraversalSettings) const;

	// 打印 Traversal Check Result
	void DebugPrintTraversalCheckResult(
		const FGAFTraversalCheckResult& TraversalCheckResult,
		const FGAFTraversalSettings& TraversalSettings) const;

protected:
	ACharacter* GetOwnerCharacter() const;

	UPROPERTY(BlueprintReadOnly)
	bool bDoingTraversalAction{ false };

	// 缓存的用于翻越系统的数据
	UPROPERTY(BlueprintReadOnly, Transient)
	FGAFTraversalFrameData CachedTraversalData;
};
