#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

struct FGAFTraversalSettings;

class GAF_API FGAFTraversalCollisionResolver
{
public:
	static ECollisionChannel GetTraversalCollisionChannel();
	static ECollisionChannel GetTraversalCollisionChannel(const FGAFTraversalSettings* TraversalSettings);
};
