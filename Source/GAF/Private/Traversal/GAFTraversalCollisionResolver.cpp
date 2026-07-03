#include "Traversal/GAFTraversalCollisionResolver.h"

#include "GAFLogChannels.h"
#include "Settings/GAFTraversalSettings.h"

namespace
{
	// 用Visibility作为fallback
	constexpr ECollisionChannel GAFTraversalFallbackChannel{ ECC_Visibility };
} // namespace

// 读取 TraversalSettings 里的配置，返回 C++ trace 使用的 Collision Channel。
ECollisionChannel FGAFTraversalCollisionResolver::GetTraversalCollisionChannel()
{
	const UGAFTraversalSettings* Settings = GetDefault<UGAFTraversalSettings>();
	if (!IsValid(Settings))
	{
		UE_LOG(LogGAFTraversal, Warning, TEXT("GAF TraversalSettings is invalid, using Visibility as fallback trace channel."));
		return GAFTraversalFallbackChannel;
	}

	return Settings->TraversableTraceChannel.GetValue();
}
