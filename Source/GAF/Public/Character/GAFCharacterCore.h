#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GAFCharacterCore.generated.h"

class UGAFCharacterMovementComponent;

UCLASS()
class GAF_API AGAFCharacterCore : public ACharacter
{
	GENERATED_BODY()

public:
	explicit AGAFCharacterCore(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 处理输入系统
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// 带有自定义逻辑的 CMC
	UPROPERTY(BlueprintReadOnly, Category = "GAF Character")
	TObjectPtr<UGAFCharacterMovementComponent> GAFCharacterMovement;
};
