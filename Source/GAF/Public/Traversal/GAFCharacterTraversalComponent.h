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
class UMotionWarpingComponent;

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

	// 真正执行 Traversal
	UFUNCTION(BlueprintCallable, Category = "GAF|Traversal")
	bool PerformTraversalAction(
		FGAFTraversalCheckResult& InOutTraversalResult,
		const FGAFTraversalSettings& TraversalSettings);

	// 更新用于 MotionWarping 组件的 Warptargets
	UFUNCTION(BlueprintCallable, Category = "GAF|Traversal")
	void UpdateWarpTargets(
		const FGAFTraversalCheckResult& TraversalResult,
		const FGAFTraversalSettings& TraversalSettings);

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

	// Montage 委托绑定
	// 参数需要与委托签名一致
	void HandleTraversalMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	void HandleTraversalMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void FinishTraversalAction();

	void UpdateFrontLedgeWarpTarget(
		UMotionWarpingComponent& MotionWarping,
		const FGAFTraversalCheckResult& TraversalResult,
		const FGAFTraversalSettings& TraversalSettings,
		const ACharacter& Character) const;

	bool UpdateBackLedgeWarpTarget(
		UMotionWarpingComponent& MotionWarping,
		const FGAFTraversalCheckResult& TraversalResult,
		const FGAFTraversalSettings& TraversalSettings,
		const UAnimMontage& ChosenMontage,
		float& OutAnimatedDistanceFromFrontLedgeToBackLedge) const;

	void UpdateBackFloorWarpTarget(
		UMotionWarpingComponent& MotionWarping,
		const FGAFTraversalCheckResult& TraversalResult,
		const FGAFTraversalSettings& TraversalSettings,
		const UAnimMontage& ChosenMontage,
		bool bHasAnimatedDistanceFromFrontLedgeToBackLedge,
		float AnimatedDistanceFromFrontLedgeToBackLedge) const;

	UPROPERTY(BlueprintReadOnly, Category = "GAF|Traversal")
	bool bDoingTraversalAction{ false };

	// 缓存的用于翻越系统的数据
	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Traversal")
	FGAFTraversalFrameData CachedTraversalData;

	// 保存 CheckResult 以供蒙太奇回调使用
	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Traversal")
	FGAFTraversalCheckResult ActiveTraversalResult;
};
