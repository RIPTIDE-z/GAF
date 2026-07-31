#pragma once

#include "CoreMinimal.h"
#include "GAFGamePlayTag.h"
#include "GameFramework/Character.h"
#include "Animation/GAFCharacterDataProvider.h"
#include "Animation/GAFAnimationTypes.h"
#include "Settings/GAFCharacterSettings.h"
#include "Traversal/GAFTraversalTypes.h"
#include "GAFCharacterCore.generated.h"

class UGAFCharacterMovementComponent;
class UGAFCharacterTraversalComponent;
class UMotionWarpingComponent;
class UCharacterMovementComponent;

UCLASS()
class GAF_API AGAFCharacterCore :
	public ACharacter,
	public IGAFCharacterDataProvider,
	public IGAFLocomotionDataProvider
{
	GENERATED_BODY()

public:
	explicit AGAFCharacterCore(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// 处理输入系统
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 组件获取
	UGAFCharacterMovementComponent* GetGAFCharacterMovement() const { return GAFCharacterMovement; }
	UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }
	UGAFCharacterTraversalComponent* GetCharacterTraversalComponent() const { return CharacterTraversalComponent; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	TObjectPtr<const UGAFCharacterSettings> CharacterSettings;

	// ---- IGAFCharacterDataProvider 的 C++ 默认实现 ----
	// 蓝图子类可覆写这些 _Implementation 函数改变数据组装逻辑
	virtual bool GetAnimationFrameData_Implementation(FGAFAnimationFrameData& OutData) override;
	virtual bool GetCameraFrameData_Implementation(FGAFCameraFrameData& OutData) override;
	virtual bool GetTraversalFrameData_Implementation(FGAFTraversalFrameData& OutData) override;

	// ---- IGAFLocomotionDataProvider 的 C++ 默认实现 ----
	virtual bool GetLocomotionData_Implementation(FGAFLocomotionData& OutData) override;

	// 根据传入的bActive修改输入Tag
	UFUNCTION(BlueprintCallable, Category = "GAF|Input")
	void SetInputStateTag(FGameplayTag Tag, bool bActive);

	// 切换输入Tag
	UFUNCTION(BlueprintCallable, Category = "GAF|Input")
	void ToggleInputStateTag(FGameplayTag Tag);

	bool HasInputStateTag(FGameplayTag Tag) const;

	// 计算当前输入状态下允许的最"大"Gait
	FGameplayTag CalculateMaxAllowedGait() const;

protected:
	bool CanSprint() const;
	void RefreshCrouchFromInputState();
	FGAFTraversalCheckInputs GetTraversalCheckInputs() const;
	const UGAFCharacterSettings& GetDefaultCharacterSettings() const;
	void InitCharacterMovementSettings(UGAFCharacterMovementComponent* CMC, const FGAFMovementSettings& Settings) const;

	// 计算移动方向在速度配置中的位置：0 = 前进，1 = 横移，2 = 后退。
	float CalculateDirectionAmount(const UCharacterMovementComponent& CMC) const;

	// 根据方向位置在 Forward / Strafe / Backward 三个速度之间插值。
	float CalculateDirectionDependentSpeed(const FVector& Speeds, float DirectionAmount) const;

protected:
	// 带有自定义逻辑的 CMC
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Character")
	TObjectPtr<UGAFCharacterMovementComponent> GAFCharacterMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Character")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	// 翻越逻辑组件，对应AC_TraversalLogic
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Character")
	TObjectPtr<UGAFCharacterTraversalComponent> CharacterTraversalComponent;

protected:
	// 输入Tag集合
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Input")
	FGameplayTagContainer InputStateTags;

	// 摇杆推动幅度，0 ~ 1，1 表示满输入
	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Input")
	float MoveInputLength{ ForceInit };

	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Input")
	float MoveWorldSpaceInputLength{ ForceInit };

	// 允许 Sprint 时移动方向和角色朝向的夹角阈值
	UPROPERTY(BlueprintReadOnly, Transient, Category = "GAF|Input")
	float SprintAngleThreshold{ 50.0f };

	// 当前相机视角风格，由输入或游戏逻辑切换
	UPROPERTY(BlueprintReadWrite, Transient, Category = "GAF|Camera")
	EGAFCameraStyle CurrentCameraStyle{ EGAFCameraStyle::Explore };

	// 当前相机左右偏好侧，由输入或游戏逻辑切换
	UPROPERTY(BlueprintReadWrite, Transient, Category = "GAF|Camera")
	EGAFCameraSide CurrentCameraSide{ EGAFCameraSide::Right };
};
