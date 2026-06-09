#include "Character/GAFCharacterCore.h"

#include "GAFCharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAFCharacterCore)

// Core character type used by the framework
AGAFCharacterCore::AGAFCharacterCore(const FObjectInitializer& ObjectInitializer)
	: Super{
		ObjectInitializer.SetDefaultSubobjectClass<UGAFCharacterMovementComponent>(CharacterMovementComponentName)
	}
{
	// Tick every frame so character state can be updated by framework systems
	PrimaryActorTick.bCanEverTick = true;

	GAFCharacterMovement = Cast<UGAFCharacterMovementComponent>(GetCharacterMovement());
}

// Called when the game starts or when spawned
void AGAFCharacterCore::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
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
