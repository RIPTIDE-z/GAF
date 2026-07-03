#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/GAFCharacterDataProvider.h"
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
		EDrawDebugTrace::Type TraversalDebugType,
		FGAFTraversalCheckResult& InOutTraversalCheckResult);

	// 输出 Traversal 检测失败原因，集中放在 Traversal 系统里，避免 Playable 直接处理调试格式。
	void DebugPrintTraversalFailureReason(
		const FGAFTraversalCheckResult& TraversalCheckResult,
		const FGAFTraversalSettings& TraversalSettings) const;

protected:
	ACharacter* GetOwnerCharacter() const;

	UPROPERTY(BlueprintReadOnly)
	bool bDoingTraversalAction{ false };

	// 缓存的运动数据
	UPROPERTY(BlueprintReadOnly, Transient)
	FGAFTraversalFrameData CachedTraversalData;
};
