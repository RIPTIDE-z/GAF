#pragma once

#include "CoreMinimal.h"
#include "GAFGamePlayTag.h"
#include "GameFramework/Character.h"
#include "Animation/GAFCharacterDataProvider.h"
#include "Animation/GAFAnimationTypes.h"
#include "GAFCharacterCore.generated.h"

class UGAFCharacterMovementComponent;
class UMotionWarpingComponent;

UCLASS()
class GAF_API AGAFCharacterCore :
	public ACharacter,
	public IGAFCharacterDataProvider,
	public IGAFLocomotionDataProvider
{
	GENERATED_BODY()

public:
	explicit AGAFCharacterCore(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// 处理输入系统
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// CMC获取
	UGAFCharacterMovementComponent* GetGAFCharacterMovement() const{return GAFCharacterMovement;}

	// 获取动画所需数据的接口
	virtual bool GetAnimationFrameData(FGAFAnimationFrameData& OutData) const override;
	virtual bool GetCameraFrameData(FGAFCameraFrameData& OutData) const override;
	virtual bool GetTraversalFrameData(FGAFTraversalFrameData& OutData) const override;

	// CMC数据传递
	virtual bool GetLocomotionData(FGAFLocomotionData& OutData) const override;

	// 根据传入的bActive修改输入Tag
	UFUNCTION(BlueprintCallable)
	void SetInputStateTag(FGameplayTag Tag, bool bActive);

	// 切换输入Tag
	UFUNCTION(BlueprintCallable)
	void ToggleInputStateTag(FGameplayTag Tag);

	bool HasInputStateTag(FGameplayTag Tag) const;

	// 计算当前输入状态下允许的最“大”Gait
	FGameplayTag CalculateMaxAllowedGait() const;

protected:
	virtual void BuildAnimationFrameData(FGAFAnimationFrameData& OutData) const;
	
	bool CanSprint() const;

protected:
	// 带有自定义逻辑的 CMC
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UGAFCharacterMovementComponent> GAFCharacterMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

public:
	UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }

protected:
	// 输入Tag集合
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Input")
	FGameplayTagContainer InputStateTags;

	// 摇杆推动幅度，0 ~ 1，1 表示满输入
	UPROPERTY(Transient, BlueprintReadOnly, Transient)
	float MoveInputLength{ ForceInit };

	UPROPERTY(Transient, BlueprintReadOnly, Transient)
	float MoveWorldSpaceInputLength{ ForceInit };
	
	// 允许 Sprint 时移动方向和角色朝向的夹角阈值
	UPROPERTY(Transient, BlueprintReadOnly, Transient)
	float SprintAngleThreshold{ 50.0f };
};
