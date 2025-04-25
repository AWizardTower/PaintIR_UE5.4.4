// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PointComponent.h"
#include "Blueprint/UserWidget.h"
#include "KeyPointUserWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValueCommitted, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKeyPointValueChanged, FVector, PointPosition, float, NewValue);

UCLASS()
class C_PAINTIR_API UKeyPointUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category = "KeyPoint")
	FOnValueCommitted OnValueCommitted;

	// 通过 BlueprintCallable 函数桥接广播
	UFUNCTION(BlueprintCallable)
	void CommitValue(float NewValue);
	
	// 所属关键点的位置（你需要在创建Widget时设置这个值）
	UPROPERTY(BlueprintReadWrite, Category="KeyPoint")
	FVector KeyPointPosition;
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void SetValue(float NewValue);
    
    // 修改值（用户输入后）
    UFUNCTION(BlueprintCallable)
    void ModifyValue(float NewValue);

	// 绑定删除按钮
    UFUNCTION(BlueprintCallable)
    void RemovePoint();

	UPROPERTY(BlueprintReadWrite, Category="KeyPoint")
	UPointComponent* OwningPoint;

private:
	UPROPERTY(EditAnywhere)
	UMaterialInterface* SphereMaterial;
};
