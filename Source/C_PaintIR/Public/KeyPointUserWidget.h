// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KeyPointUserWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKeyPointValueChanged, FVector, PointPosition, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKeyPointRemoved, FVector, PointPosition);
/**
 * 
 */
UCLASS()
class C_PAINTIR_API UKeyPointUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 所属关键点的位置（你需要在创建Widget时设置这个值）
	UPROPERTY(BlueprintReadWrite, Category="KeyPoint")
	FVector KeyPointPosition;

	// 通知外部：用户修改了值
	UPROPERTY(BlueprintAssignable, Category="KeyPoint")
	FOnKeyPointValueChanged OnValueChanged;

	// 通知外部：用户希望删除这个点
	UPROPERTY(BlueprintAssignable, Category="KeyPoint")
	FOnKeyPointRemoved OnRemoveRequested;
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void SetValue(float NewValue);
    
    	// 修改值（用户输入后）
    	UFUNCTION(BlueprintCallable)
    	void ModifyValue(float NewValue);
	
		// 绑定删除按钮
    	UFUNCTION(BlueprintCallable)
    	void RemovePoint();
};
