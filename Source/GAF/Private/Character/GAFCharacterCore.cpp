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

	// 1.1 Orient Intent
	bool bWantAim =
		HasInputStateTag(GAFGamePlayTags::InputState_WantsToAim)
		|| HasInputStateTag(GAFGamePlayTags::InputState_WantsToSprint);

	// 1.2 Rotation Rate
	// 2. Update Movement
	// 2.1 Gait
	// 2.2 Max Acceleration
	// 2.3 Braking Deceleration
	// 2.4 Ground Friction
	// 2.5 Max Walk Speed
	// 2.6 Max Walk Speed Crouched
	return true;
}

// 实际获取动画数据，从CMC中直接获取
void AGAFCharacterCore::BuildAnimationFrameData(FGAFAnimationFrameData& OutData) const
{
}

bool AGAFCharacterCore::HasInputStateTag(FGameplayTag Tag) const
{
	return Tag.IsValid() && InputStateTags.HasTagExact(Tag);
}

FGameplayTag AGAFCharacterCore::CalculateMaxAllowedGait() const
{
	// 如果有冲刺状态则优先冲刺
	if (HasInputStateTag(GAFGamePlayTags::InputState_WantsToSprint))
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