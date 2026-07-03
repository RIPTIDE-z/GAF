#include "Traversal/GAFTraversalCollisionResolver.h"

#include "GAFLogChannels.h"
#include "Settings/GAFTraversalConfig.h"
#include "Settings/GAFTraversalSettings.h"

namespace
{
	// 用Visibility作为fallback
	constexpr ECollisionChannel GAFTraversalFallbackChannel{ ECC_Visibility };
} // namespace

// 读取全局 TraversalConfig 里的配置，返回 C++ trace 使用的 Collision Channel
ECollisionChannel FGAFTraversalCollisionResolver::GetTraversalCollisionChannel()
{
	const UGAFTraversalConfig* Config = GetDefault<UGAFTraversalConfig>();
	if (!IsValid(Config))
	{
		UE_LOG(LogGAFTraversal, Warning, TEXT("GAF TraversalConfig is invalid, using Visibility as fallback trace channel."));
		return GAFTraversalFallbackChannel;
	}

	return Config->TraversableTraceChannel.GetValue();
}

// 角色配置开启覆盖时优先使用角色自己的 Channel，否则回退到项目级配置
ECollisionChannel FGAFTraversalCollisionResolver::GetTraversalCollisionChannel(const FGAFTraversalSettings* TraversalSettings)
{
	if (TraversalSettings != nullptr && TraversalSettings->bOverrideTraversalTraceChannel)
	{
		return TraversalSettings->TraversalTraceChannel.GetValue();
	}

	return GetTraversalCollisionChannel();
}
