#include "Character/GAFCharacterCore.h"

#include "GAFLogChannels.h"
#include "Component/GAFCharacterMovementComponent.h"
#include "MotionWarpingComponent.h"
#include "GAFGamePlayTag.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFCharacterCore)

// Core character type used by the framework
// 框架使用的核心角色类
AGAFCharacterCore::AGAFCharacterCore(const FObjectInitializer& ObjectInitializer)
	: Super{
		// 使用自定义CMC覆盖默认CMC
		ObjectInitializer.SetDefaultSubobjectClass<UGAFCharacterMovementComponent>(CharacterMovementComponentName)
	}
{
	PrimaryActorTick.bCanEverTick = true;

	GAFCharacterMovement = Cast<UGAFCharacterMovementComponent>(GetCharacterMovement());

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));
}

void AGAFCharacterCore::BeginPlay()
{
	Super::BeginPlay();
}

void AGAFCharacterCore::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGAFCharacterCore::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AGAFCharacterCore::SetInputStateTag(FGameplayTag Tag, bool bActive)
{
	if (!Tag.IsValid())
	{
		UE_LOG(LogGAFCore, Warning, TEXT("Can't SetInputStateTag for Tag [%s] on [%s]."), *Tag.ToString(), *GetNameSafe(this));
		return;
	}

	if (bActive)
	{
		InputStateTags.AddTag(Tag);
	}
	else
	{
		InputStateTags.RemoveTag(Tag);
	}
}

void AGAFCharacterCore::ToggleInputStateTag(FGameplayTag Tag)
{
	if (!Tag.IsValid())
	{
		UE_LOG(LogGAFCore, Warning, TEXT("Can't ToggleInputStateTag for Tag [%s] on [%s]."), *Tag.ToString(), *GetNameSafe(this));
		return;
	}

	if (InputStateTags.HasTagExact(Tag))
	{
		InputStateTags.RemoveTag(Tag);
	}
	else
	{
		InputStateTags.AddTag(Tag);
	}
}

bool AGAFCharacterCore::GetAnimationFrameData(FGAFAnimationFrameData& OutData) const
{
	return false;
}

bool AGAFCharacterCore::GetCameraFrameData(FGAFCameraFrameData& OutData) const
{
	return false;
}

bool AGAFCharacterCore::GetTraversalFrameData(FGAFTraversalFrameData& OutData) const
{
	return false;
}

bool AGAFCharacterCore::GetLocomotionData(FGAFLocomotionData& OutData) const
{
	// 1. Update Rotation
	/**
	 * 该函数根据角色当前是否希望面向 Controller 方向，
	 * 更新 Character Movement Component 的旋转模式
	 *
	 * 当角色处于 Aim 或 Strafe 状态时，角色的目标朝向来自 Controller Rotation
	 * 此时角色可以保持面向瞄准方向或镜头方向，同时由移动输入控制前后左右平移
	 *
	 * 当角色不处于 Aim 或 Strafe 状态时，角色的目标朝向来自移动方向
	 * 此时角色会根据移动输入自动转向，例如向前跑、侧向转身跑或快速改变移动方向
	 *
	 * 当角色处于地面状态时，Rotation Rate 设置为 -1，
	 * 使 Actor 能够立即旋转到当前计算出的目标朝向
	 *
	 * 该方案的核心思想是：
	 * 将 Actor Rotation 作为角色的 “ 逻辑目标朝向 ”，而不是直接作为最终的视觉朝向
	 * 最终玩家看到的身体旋转表现，由 Animation Blueprint 中的 Offset Root Bone 旋转单独控制
	 *
	 * 这样可以绕过 Character Movement Component 当前旋转控制能力的限制，
	 * 实现更复杂的角色朝向行为，例如：
	 *    Stick Flick：玩家短暂点击或轻拨移动输入时，也能让角色立即重新朝向新的移动方向
	 *    Turn Start：角色从静止或低速状态开始转身时，可以由动画控制起转过程
	 *    Pivot：角色高速移动中反向或大角度变向时，可以由动画控制转身和脚步表现
	 *    Turn In Place：角色原地转身时，可以独立控制身体旋转，而不是简单旋转整个 Actor
	 *
	 * 当前方案仍属于实验性实现
	 * 后续引擎版本中的新 Movement Component 预计会提供更完善的旋转控制能力
	 */
	UCharacterMovementComponent* CMC = this->GetCharacterMovement();
	if (!CMC)
	{
		UE_LOG(LogGAFCore, Warning, TEXT("Can Get Locomotion Data failed, CMC is invalid on [%s]."), *GetNameSafe(this));
		return false;
	}
	
	// 提前获取平面速度
	const float Speed = CMC->Velocity.Size2D();

	// 1.1 Orient Intent
	// Aim / Strafe 时，角色朝向 Controller方向
	// 普通移动时则朝向移动输入方向
	bool bShouldFaceController =
		HasInputStateTag(GAFGamePlayTags::InputState_WantsToAim)
		|| HasInputStateTag(GAFGamePlayTags::InputState_WantsToStrafe);
	
	OutData.bUseControllerDesiredRotation = bShouldFaceController;
	OutData.bOrientRotationToMovement = !bShouldFaceController;

	// 1.2 Rotation Rate
	// TODO: 参数化加进 MovementSettings
	if (CMC->IsFalling())
	{
		OutData.RotationRate = FRotator(0.0, 0.0, 200.0);
	}
	else
	{
		OutData.RotationRate = FRotator(0.0, 0.0, -1.0);
	}
	
	// 2. Update Movement
	// 2.1 Gait
	FGameplayTag MaxAllowedGait = CalculateMaxAllowedGait();
	
	// 2.2 Max Acceleration
	// TODO: 参数化加进 MovementSettings，Map可以用Curve
	// Run/Walk 固定加速度(直接默认返回这个固定值)
	// Sprint 覆盖固定值，并且时速度越快，加速度越低
	OutData.MaxAcceleration = 800.0f;
	
	if (MaxAllowedGait == GAFGamePlayTags::Gait_Sprint)
	{
		OutData.MaxAcceleration = FMath::GetMappedRangeValueClamped(
			FVector2D(300.0f, 700.0f),
			FVector2D(800.0f, 300.0f),
			Speed
		);
	}
	
	// 2.3 Braking Deceleration
	// TODO: 参数化加进 MovementSettings
	// 有输入时快速刹车提高响应度，无输入时慢速刹车
	const bool bHasMovementInputVector = !GetPendingMovementInputVector().IsNearlyZero();
	if (bHasMovementInputVector)
	{
		OutData.BrakingDecelerationWalking = 500.0f;
	}
	else
	{
		OutData.BrakingDecelerationWalking = 2000.0f;
	}
	
	// 2.4 Ground Friction
	// Run/Walk 固定地面摩擦力
	// Sprint时速度越快，地面摩擦力越低
	OutData.GroundFriction = 5.0f;

	if (MaxAllowedGait == GAFGamePlayTags::Gait_Sprint)
	{
		OutData.GroundFriction = FMath::GetMappedRangeValueClamped(
			FVector2D(0.0f, 500.0f),
			FVector2D(5.0f, 3.0f),
			Speed
		);
	}
	
	// 2.5 Max Walk Speed
	// This function is used to set the max speed for the character’s movement. 
	// Because the forwards, strafes, and backwards animations move at different speeds, 
	// we need to change the max speed of the character based on its movement direction. 
	// We use a simple curve to map different speed values to the different directions. 
	// 0 = forward, 1 = strafe L or R, 2 = Backwards.
	
	
	// 2.6 Max Walk Speed Crouched
	
	
	return true;
}

// 实际获取动画数据，从CMC中直接获取
void AGAFCharacterCore::BuildAnimationFrameData(FGAFAnimationFrameData& OutData) const
{
}

bool AGAFCharacterCore::CanSprint() const
{
	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!IsValid(Movement))
	{
		UE_LOG(LogGAFCore, Warning, TEXT("CanSprint judge failed, CMC is invalid on [%s]."), *GetNameSafe(this));
		return false;
	}

	const bool bWantsToSprint =
		HasInputStateTag(GAFGamePlayTags::InputState_WantsToSprint);

	const bool bWantsToCrouch =
		HasInputStateTag(GAFGamePlayTags::InputState_WantsToCrouch);

	if (!bWantsToSprint || bWantsToCrouch)
	{
		return false;
	}

	if (Movement->bOrientRotationToMovement)
	{
		return true;
	}

	// 本地角色使用输入方向
	// 远端角色则使用 CMC 的加速度方向
	const FVector MovementIntent =
		IsLocallyControlled()
			? GetPendingMovementInputVector()
			: Movement->GetCurrentAcceleration();

	if (MovementIntent.IsNearlyZero())
	{
		return false;
	}

	// 角色朝向
	const FRotator ActorRotation = GetActorRotation();
	// 运动方向
	const FRotator MovementRotation = MovementIntent.ToOrientationRotator();

	const float DeltaYaw = FMath::Abs(
		FRotator::NormalizeAxis(MovementRotation.Yaw - ActorRotation.Yaw)
	);

	// 夹角小于阈值才能移动
	return DeltaYaw < SprintAngleThreshold;
}

bool AGAFCharacterCore::HasInputStateTag(FGameplayTag Tag) const
{
	return Tag.IsValid() && InputStateTags.HasTagExact(Tag);
}

FGameplayTag AGAFCharacterCore::CalculateMaxAllowedGait() const
{
	//TODO: Movement Stick Mode
	
	// 如果允许冲刺状态则优先冲刺
	if (CanSprint())
	{
		return GAFGamePlayTags::Gait_Sprint;
	}

	if (HasInputStateTag(GAFGamePlayTags::InputState_WantsToWalk))
	{
		return GAFGamePlayTags::Gait_Walk;
	}

	// 默认Run
	return GAFGamePlayTags::Gait_Run;
}