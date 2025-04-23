// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "MyUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class C_PAINTIR_API UMyUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 在外部设置贴图
	UFUNCTION(BlueprintCallable)
	void SetRenderTargetToImage(UTextureRenderTarget2D* RenderTarget);

protected:
	// 绑定蓝图中的 Image 控件
	UPROPERTY(meta = (BindWidget))
	UImage* MyImage;
};
