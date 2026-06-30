#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Animation/GAFCharacterDataProvider.h"
#include "Animation/GAFAnimationTypes.h"
#include "GAFCharacterCore.generated.h"

class UGAFCharacterMovementComponent;

UCLASS()
class GAF_API AGAFCharacterCore : public ACharacter, public IGAFCharacterDataProvider
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

	// 获取动画所需数据的接口
	virtual bool GetAnimationFrameData(FGAFAnimationFrameData& OutData) const override;
	virtual bool GetCameraFrameData(FGAFCameraFrameData& OutData) const override;
	virtual bool GetTraversalFrameData(FGAFTraversalFrameData& OutData) const override;

	// 增删输入Tag
	UFUNCTION(BlueprintCallable, Category = "GAF|Input")
	void SetInputStateTag(FGameplayTag Tag, bool bActive);

protected:
	virtual void BuildAnimationFrameData(FGAFAnimationFrameData& OutData) const;
	
protected:
	// 带有自定义逻辑的 CMC
	UPROPERTY(BlueprintReadOnly, Category = "GAF|Character")
	TObjectPtr<UGAFCharacterMovementComponent> GAFCharacterMovement;

	// 输入状态
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAF|Character")
	FGameplayTagContainer InputStateTags;

};
