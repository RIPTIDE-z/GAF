#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "State/GAFALSViewState.h"
#include "GAFALSCharacterCore.generated.h"

class UGAFALSCharacterMovementComponent;

UCLASS()
class GAFALS_API AGAFALSCharacterCore : public ACharacter
{
	GENERATED_BODY()

public:
	explicit AGAFALSCharacterCore(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 绑定输入
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	const FGAFALSViewState& GetViewState() const;

public:
	virtual FRotator GetViewRotation() const override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "GAFALS Character")
	TObjectPtr<UGAFALSCharacterMovementComponent> GAFALSCharacterMovement;
	// 存储视角信息
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|GAFALS Character", Transient)
	FGAFALSViewState ViewState;
};

inline const FGAFALSViewState& AGAFALSCharacterCore::GetViewState() const
{
	return ViewState;
}
