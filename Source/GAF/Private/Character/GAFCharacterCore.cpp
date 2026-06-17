#include "Character/GAFCharacterCore.h"

#include "GAFCharacterMovementComponent.h"

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
}

void AGAFCharacterCore::BeginPlay()
{
	Super::BeginPlay();
}

void AGAFCharacterCore::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AGAFCharacterCore::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

FRotator AGAFCharacterCore::GetViewRotation() const
{
	// TODO:ViewState
	// return ViewState.Rotation;

	return GetController() ? GetController()->GetControlRotation() : Super::GetViewRotation();
}
