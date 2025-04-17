// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class C_PAINTIR_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMyPlayerController();

	// 当前绘制属性值
	float CurrentValue;

	// 设置绘制属性值
	// 这么说还必须设置为蓝图可调用的
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetCurrentValue(float NewValue);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> UMGMainWidgetClass;

	// 移动角色到指定位置并旋转
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MoveCharacter(const FVector& TargetLocation, const FRotator& TargetRotation);

private:
	void HandleLeftClick();

protected:
	virtual void SetupInputComponent() override;

	virtual void BeginPlay() override;

};
