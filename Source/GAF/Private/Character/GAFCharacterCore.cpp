#include "Character/GAFCharacterCore.h"

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
	
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
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