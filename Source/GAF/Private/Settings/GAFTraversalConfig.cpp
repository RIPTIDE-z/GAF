#include "Settings/GAFTraversalConfig.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFTraversalConfig)

UGAFTraversalConfig::UGAFTraversalConfig()
{
	// 未配置项目专用 Channel 时，用 Visibility 作为安全兜底。
	TraversableTraceChannel = ECC_Visibility;
}

FName UGAFTraversalConfig::GetCategoryName() const
{
	return TEXT("Plugins");
}
