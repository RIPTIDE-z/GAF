#pragma once

#include "GameFramework/CharacterMovementComponent.h"
//#include "Settings/GAFALSMovementSettings.h"
#include "GAFALSCharacterMovementComponent.generated.h"

UCLASS(ClassGroup = "GAFALS")
class GAFALS_API UGAFALSCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UGAFALSCharacterMovementComponent();

	virtual FVector ConsumeInputVector() override;

	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;

public:
	virtual void PhysicsRotation(float DeltaTime) override;

protected:
	virtual void PhysWalking(float DeltaTime, int32 IterationsCount) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Meta = (ClampMin = 0, ForceUnits = "cm/s^2"))
	float MaxAccelerationWalking = 0.0f;

	// 用于暂时禁用玩家移动角色，适用于AI控制的角色
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bInputBlocked = false;
};