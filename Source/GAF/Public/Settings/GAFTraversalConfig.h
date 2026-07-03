#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/EngineTypes.h"

#include "GAFTraversalConfig.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "GAF Traversal Config"))
class GAF_API UGAFTraversalConfig : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGAFTraversalConfig();

	virtual FName GetCategoryName() const override;

public:
	// 全局默认 Traversal Trace Channel
	// 推荐创建名为 Traversable 的 Trace Channel 后在这里选择
	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Collision")
	TEnumAsByte<ECollisionChannel> TraversableTraceChannel;
};
