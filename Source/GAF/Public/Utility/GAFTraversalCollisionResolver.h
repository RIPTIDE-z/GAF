#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

class GAF_API FGAFTraversalCollisionResolver
{
public:
	static ETraceTypeQuery GetTraversalTraceType();
	static ECollisionChannel GetTraversalCollisionChannel();
};
