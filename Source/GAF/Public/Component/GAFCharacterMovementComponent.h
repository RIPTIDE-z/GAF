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

protected:
	virtual void PhysWalking(float DeltaTime, int32 IterationsCount) override;
	// 在 PhsicsRotation 和 PhysWalking 前被调用
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	void RefreshActualGait();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Meta = (ClampMin = 0, ForceUnits = "cm/s^2"))
	float MaxAccelerationWalking = 0.0f;

	// 如果有其他系统要操作角色移动，屏蔽输入
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bInputBlocked = false;

	// 输入状态允许的最大Gait
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag MaxAllowedGait;

	// 角色的真是Gait
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag ActualGait;
};
