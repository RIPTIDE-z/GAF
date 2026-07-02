#include "Character/GAFCharacterCore.h"

#include "GAFLogChannels.h"
#include "Component/GAFCharacterMovementComponent.h"
#include "Component/GAFTraversalComponent.h"
#include "Curves/CurveFloat.h"
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
	// TODO: Crouch逻辑暂时放在Core
	if (IsValid(GAFCharacterMovement))
	{
		// Crouch() 需要 CMC 允许蹲伏，否则输入状态改变后角色不会真正进入蹲伏状态。
		GAFCharacterMovement->GetNavAgentPropertiesRef().bCanCrouch = true;
	}

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));
	TraversalComponent = CreateDefaultSubobject<UGAFTraversalComponent>(TEXT("Traversal"));
}

void AGAFCharacterCore::PostInitializeComponents()
{
	// 不在构造函数初始化设定，防止CDO阶段读到空DataAsset
	Super::PostInitializeComponents();

	const UGAFCharacterSettings& Settings = GetDefaultCharacterSettings();

	SetInputStateTag(GAFGamePlayTags::InputState_WantsToStrafe, Settings.MovementSettings.DefaultStrafe);
	InitCharacterMovementSettings(GAFCharacterMovement, Settings.MovementSettings);
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
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!IsValid(Capsule))
	{
		UE_LOG(LogGAFTraversal, Warning, TEXT("Get Traversal Frame Data failed, Capsule is invalid on [%s]."), *GetNameSafe(this));
		return false;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!IsValid(MeshComponent))
	{
		UE_LOG(LogGAFTraversal, Warning, TEXT("Get Traversal Frame Data failed, Mesh is invalid on [%s]."), *GetNameSafe(this));
		return false;
	}

	UMotionWarpingComponent* MotionWarping = GetMotionWarpingComponent();
	if (!IsValid(MotionWarping))
	{
		UE_LOG(LogGAFTraversal, Warning, TEXT("Get Traversal Frame Data failed, MotionWarpingComponent is invalid on [%s]."), *GetNameSafe(this));
		return false;
	}

	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (!IsValid(CMC))
	{
		UE_LOG(LogGAFTraversal, Warning, TEXT("Get Traversal Frame Data failed, CMC is invalid on [%s]."), *GetNameSafe(this));
		return false;
	}

	OutData.Capsule = Capsule;
	OutData.Mesh = MeshComponent;
	OutData.MotionWarping = MotionWarping;
	OutData.Speed = CMC->Velocity.Size2D();
	
	// TODO:这里基于现有方案进行了一次重复的Gait计算，应该想办法与CMC的更新结合减少计算次数
	OutData.Gait = CalculateMaxAllowedGait();

	// 只有 Falling Swimming 判定为在空中
	switch (CMC->MovementMode)
	{
		case EMovementMode::MOVE_Falling:
			OutData.MovementMode = GAFGamePlayTags::MovementMode_InAir;
			break;
		case EMovementMode::MOVE_Swimming:
			OutData.MovementMode = GAFGamePlayTags::MovementMode_InAir;
			break;
		default:
			OutData.MovementMode = GAFGamePlayTags::MovementMode_OnGround;
			break;
	}

	return true;
}

bool AGAFCharacterCore::GetLocomotionData(FGAFLocomotionData& OutData) const
{
	const UGAFCharacterSettings& CharacterSetting = GetDefaultCharacterSettings();
	const FGAFMovementSettings& MovementSettings = CharacterSetting.MovementSettings;

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
			Speed);
	}

	// 2.3 Braking Deceleration
	// TODO: 参数化加进 MovementSettings
	// 有输入时快速刹车提高响应度，无输入时慢速刹车
	const bool bHasMovementInputVector = !GetLastMovementInputVector().IsNearlyZero();
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
			Speed);
	}

	/**
	 * 2.5 Max Walk Speed
	 *
	 * 角色的前进、横移、后退动画通常使用不同的参考速度
	 * 如果只给 Character Movement Component 一个固定 MaxWalkSpeed，
	 * 角色物理速度就可能和当前方向上的动画速度不匹配
	 *
	 * 因此这里先计算移动意图方向与角色朝向之间的绝对 Yaw 夹角，
	 * 再通过 StrafeSpeedMapCurve 将角度映射成方向速度索引
	 *
	 * 0 表示前进速度，1 表示左/右横移速度，2 表示后退速度
	 *
	 * 最终根据 MaxAllowedGait 选择 Walk / Run / Sprint 的速度组，
	 * 并用方向速度索引在 Forward / Strafe / Backward 三个速度之间插值
	 */

	// 将移动意图方向与角色朝向夹角映射为自定义的索引 DirectionAmount
	// 0 表示前进，1 表示左/右横移，2 表示后退
	// 同时适用于 Stand Crouch 以及可能未来加入的 Prone
	// 之后会结合曲线做速度映射
	const float DirectionAmount = CalculateDirectionAmount(*CMC);

	// MaxAllowedGait 决定当前使用哪一组速度(Walk/Run.Sprint)
	// 具体前进、横移、后退取哪一个速度由 DirectionAmount 再细分
	const FVector* GaitSpeeds = &MovementSettings.RunSpeeds;
	if (MaxAllowedGait == GAFGamePlayTags::Gait_Walk)
	{
		GaitSpeeds = &MovementSettings.WalkSpeeds;
	}
	else if (MaxAllowedGait == GAFGamePlayTags::Gait_Sprint)
	{
		GaitSpeeds = &MovementSettings.SprintSpeeds;
	}

	// 做速度映射
	OutData.MaxWalkSpeed = CalculateDirectionDependentSpeed(*GaitSpeeds, DirectionAmount);

	// 2.6 Max Walk Speed Crouched
	// 蹲伏复用同一套方向映射逻辑，只是速度组换成 CrouchSpeeds。
	OutData.MaxWalkSpeedCrouched = CalculateDirectionDependentSpeed(MovementSettings.CrouchSpeeds, DirectionAmount);

	return true;
}

// 判断是否允许冲刺
bool AGAFCharacterCore::CanSprint() const
{
	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!IsValid(Movement))
	{
		UE_LOG(LogGAFCore, Warning, TEXT("CanSprint judge failed, CMC is invalid on [%s]."), *GetNameSafe(this));
		return false;
	}

	// 先获取输入意图
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

	// 玩家的运动意图方向
	// 本地角色使用输入方向
	// 远端角色则使用 CMC 的加速度方向
	// 且需要注意的是这里需要使用 GetLastMovementInputVector
	// 因为这里是在CMC内部获取输入，在这之前CMC会消耗InputVector
	// Pending 会始终为0
	const FVector MovementIntent =
		IsLocallyControlled()
		? GetLastMovementInputVector()
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
		FRotator::NormalizeAxis(MovementRotation.Yaw - ActorRotation.Yaw));

	// 夹角小于阈值才能移动
	return DeltaYaw < SprintAngleThreshold;
}

// TODO: Crouch逻辑暂时放在Core
void AGAFCharacterCore::RefreshCrouchFromInputState()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!IsValid(Movement))
	{
		UE_LOG(LogGAFCore, Warning, TEXT("%s failed to refresh crouch state: CMC is invalid."), *GetNameSafe(this));
		return;
	}

	// 下落时不切换蹲伏
	if (Movement->IsFalling())
	{
		return;
	}

	const bool bWantsToCrouch = HasInputStateTag(GAFGamePlayTags::InputState_WantsToCrouch);
	if (bWantsToCrouch)
	{
		if (!bIsCrouched)
		{
			Crouch();
		}
	}
	else if (bIsCrouched)
	{
		UnCrouch();
	}
}

const UGAFCharacterSettings& AGAFCharacterCore::GetDefaultCharacterSettings() const
{
	return IsValid(CharacterSettings)
		? *CharacterSettings
		: *GetDefault<UGAFCharacterSettings>();
}

// 初始化CMC参数
void AGAFCharacterCore::InitCharacterMovementSettings(
	UGAFCharacterMovementComponent* CMC,
	const FGAFMovementSettings& Settings) const
{
	if (!IsValid(CMC))
	{
		UE_LOG(LogGAFCore, Warning, TEXT("%s's CMC is invalid, movement settings initialization will be skipped."), *GetNameSafe(this));
		return;
	}
	if (!IsValid(Settings.StrafeSpeedMapCurve))
	{
		UE_LOG(LogGAFCore, Error, TEXT("%s missing StrafeSpeedMapCurve in Init stage."), *GetNameSafe(this));
	}

	CMC->MaxAcceleration = Settings.MaxAcceleration;
	CMC->bUseSeparateBrakingFriction = Settings.UseSeparateBrakingFriction;
	CMC->BrakingDecelerationWalking = Settings.BrakingDecelerationWalking;
	CMC->BrakingFriction = Settings.BrakingFriction;
	CMC->BrakingFrictionFactor = Settings.BrakingFrictionFactor;
}

/**
 * 计算当前移动意图相对角色朝向的方向索引
 *
 * 和 GASP 的 StrafeSpeedMapCurve 保持一致：
 * 0 表示前进，1 表示左/右横移，2 表示后退
 * 如果输入或加速度为空，则回退到 Velocity，保证已有运动但当前帧无输入时仍能得到合理方向
 */
float AGAFCharacterCore::CalculateDirectionAmount(const UCharacterMovementComponent& CMC) const
{
	FVector MovementIntent = IsLocallyControlled()
		? GetLastMovementInputVector()
		: CMC.GetCurrentAcceleration();

	// 没有明确输入或加速度时，用当前速度方向兜底
	if (MovementIntent.IsNearlyZero())
	{
		MovementIntent = CMC.Velocity;
	}

	// 角色完全静止且没有输入时，按前进方向处理，避免产生无意义方向。
	if (MovementIntent.IsNearlyZero())
	{
		return 0.0f;
	}

	// 只关心水平面上的朝向差：0 度是前进，90 度是横移，180 度是后退。
	const float AbsDeltaYaw = FMath::Abs(
		FRotator::NormalizeAxis(MovementIntent.ToOrientationRotator().Yaw - GetActorRotation().Yaw));

	// 如果配置了曲线，就由曲线决定角度到方向索引的映射
	const UCurveFloat* StrafeSpeedMapCurve = GetDefaultCharacterSettings().MovementSettings.StrafeSpeedMapCurve.Get();
	if (IsValid(StrafeSpeedMapCurve))
	{
		return FMath::Clamp(StrafeSpeedMapCurve->GetFloatValue(AbsDeltaYaw), 0.0f, 2.0f);
	}

	UE_LOG(LogGAFCore, Warning, TEXT("%s's StrafeSpeedMapCurve is invalid, using fallback speed map."), *GetNameSafe(this));

	// TODO: 简化硬编码fallback逻辑和可配置性
	if (AbsDeltaYaw <= 45.0f)
	{
		return 0.0f;
	}

	if (AbsDeltaYaw <= 80.0f)
	{
		return FMath::GetMappedRangeValueClamped(
			FVector2D(45.0f, 80.0f),
			FVector2D(0.0f, 1.0f),
			AbsDeltaYaw);
	}

	if (AbsDeltaYaw <= 100.0f)
	{
		return 1.0f;
	}

	if (AbsDeltaYaw <= 135.0f)
	{
		return FMath::GetMappedRangeValueClamped(
			FVector2D(100.0f, 135.0f),
			FVector2D(1.0f, 2.0f),
			AbsDeltaYaw);
	}

	return 2.0f;
}

/**
 * 根据方向索引计算当前方向上的目标速度
 *
 * Speeds.X 是前进速度，Speeds.Y 是横移速度，Speeds.Z 是后退速度
 * DirectionAmount 处于 0..1 时，在前进和横移之间插值
 * DirectionAmount 处于 1..2 时，在横移和后退之间插值
 */
float AGAFCharacterCore::CalculateDirectionDependentSpeed(const FVector& Speeds, const float DirectionAmount) const
{
	const float ClampedDirectionAmount = FMath::Clamp(DirectionAmount, 0.0f, 2.0f);

	if (ClampedDirectionAmount <= 1.0f)
	{
		// 0 -> Forward，1 -> Strafe。
		return FMath::Lerp(Speeds.X, Speeds.Y, ClampedDirectionAmount);
	}

	// 1 -> Strafe，2 -> Backward。
	return FMath::Lerp(Speeds.Y, Speeds.Z, ClampedDirectionAmount - 1.0f);
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
