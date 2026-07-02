#include "Utility/GAFTraversalCollisionResolver.h"

#include "GAFLogChannels.h"
#include "Settings/GAFTraversalSettings.h"

namespace
{
	// 用Visibility作为fallback
	constexpr ECollisionChannel GAFTraversalFallbackChannel{ ECC_Visibility };
}

// 读取 TraversalSettings 里的配置并转换为真正使用的 Channel
ETraceTypeQuery FGAFTraversalCollisionResolver::GetTraversalTraceType()
{
	const UGAFTraversalSettings* Settings = GetDefault<UGAFTraversalSettings>();
	if (!IsValid(Settings))
	{
		UE_LOG(LogGAFTraversal, Warning,
			TEXT("GAF TraversalSettings is invalid, using Visibility as fallback trace channel."));
		return UEngineTypes::ConvertToTraceType(GAFTraversalFallbackChannel);
	}

	return Settings->TraversableTraceChannel.GetValue();
}

ECollisionChannel FGAFTraversalCollisionResolver::GetTraversalCollisionChannel()
{
	return UEngineTypes::ConvertToCollisionChannel(GetTraversalTraceType());
}
