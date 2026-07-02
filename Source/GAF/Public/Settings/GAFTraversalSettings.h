#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/EngineTypes.h"
#include "GAFTraversalSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "GAF Traversal"))
class GAF_API UGAFTraversalSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGAFTraversalSettings();

	virtual FName GetCategoryName() const override;

public:
	// 实际用于 Traversal 检测的 Channel
	// 推荐项目创建名为 Traversable 的 Trace Channel，然后在这里选择
	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Collision")
	TEnumAsByte<ECollisionChannel> TraversableTraceChannel;
};
