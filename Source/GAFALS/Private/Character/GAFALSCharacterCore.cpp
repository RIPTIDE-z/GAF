#include "Character/GAFALSCharacterCore.h"

#include "GAFALSCharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFALSCharacterCore)

// 核心角色类
AGAFALSCharacterCore::AGAFALSCharacterCore(const FObjectInitializer& ObjectInitializer)
	: Super{
		ObjectInitializer.SetDefaultSubobjectClass<UGAFALSCharacterMovementComponent>(CharacterMovementComponentName)
	}
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GAFALSCharacterMovement = Cast<UGAFALSCharacterMovementComponent>(GetCharacterMovement());
}

// Called when the game starts or when spawned
void AGAFALSCharacterCore::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AGAFALSCharacterCore::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AGAFALSCharacterCore::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

FRotator AGAFALSCharacterCore::GetViewRotation() const
{
	// TODO:ViewState
	// return ViewState.Rotation;

	return GetController() ? GetController()->GetControlRotation() : Super::GetViewRotation();
}
