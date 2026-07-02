#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "GAFTraversableLedgeSplineComponent.generated.h"

UCLASS(ClassGroup = (GAF), meta = (BlueprintSpawnableComponent))
class GAF_API UGAFTraversableLedgeSplineComponent : public USplineComponent
{
	GENERATED_BODY()

public:
	UGAFTraversableLedgeSplineComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAF|Traversal")
	bool bEnabled{ true };
};
