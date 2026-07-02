#include "Settings/GAFTraversalSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFTraversalSettings)

UGAFTraversalSettings::UGAFTraversalSettings()
{
	// Visibility兜底
	TraversableTraceChannel = ECC_Visibility;
}

// ProjectSettings中的插件分栏
FName UGAFTraversalSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}
