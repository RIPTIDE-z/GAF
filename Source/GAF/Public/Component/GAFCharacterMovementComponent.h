#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTagContainer.h"

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
	
	// 在 PhsicsRotation 和 PhysWalking 前被调用
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

protected:
	virtual void PhysWalking(float DeltaTime, int32 IterationsCount) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Meta = (ClampMin = 0, ForceUnits = "cm/s^2"))
	float MaxAccelerationWalking = 0.0f;

	// 如果有其他系统要操作角色移动，屏蔽输入
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bInputBlocked = false;

	// 输入状态允许的最大Gait，本质上不是 “ 真实 ” Gait
	// 但是 GASP 将 Walk/Run/Sprint 的过渡动画与 Loop 放在一起
	// 比如 Sprint to Run 位于 Run_Loop PSD
	// 这样不需要角色真正切换了步态也可以切换 Gait 数据库，不需要额外的 Actual Gait
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag MaxAllowedGait;
};
