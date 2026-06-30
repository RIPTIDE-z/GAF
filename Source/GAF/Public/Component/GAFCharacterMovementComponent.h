#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "GAFCharacterMovementComponent.generated.h"

UCLASS(ClassGroup = "GAF")
class GAF_API UGAFCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UGAFCharacterMovementComponent();

	virtual FVector ConsumeInputVector() override;

	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;

public:
	virtual void PhysicsRotation(float DeltaTime) override;

protected:
	virtual void PhysWalking(float DeltaTime, int32 IterationsCount) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Meta = (ClampMin = 0, ForceUnits = "cm/s^2"))
	float MaxAccelerationWalking = 0.0f;

	// Blocks movement input while another system controls movement
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bInputBlocked = false;
};
