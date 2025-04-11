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

private:
	void HandleLeftClick();

protected:
	virtual void SetupInputComponent() override;

	virtual void BeginPlay() override;

};
