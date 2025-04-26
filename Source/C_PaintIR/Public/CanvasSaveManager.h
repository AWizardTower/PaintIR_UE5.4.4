// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "KeyPointData.h"
#include "CanvasSaveManager.generated.h"

/**
 * 
 */
UCLASS()
class C_PAINTIR_API UCanvasSaveManager : public UObject
{
	GENERATED_BODY()

public:
	// 将所有模型的绘制数据保存到硬盘
	UFUNCTION(BlueprintCallable, Category="Canvas Save")
	static void SaveAllCanvasData(const TMap<FName, FKeyPointData>& AllDrawnPoints);

	// 从硬盘加载所有模型的绘制数据
	UFUNCTION(BlueprintCallable, Category="Canvas Save")
	static TMap<FName, FKeyPointData> LoadAllCanvasData();

	static FString SlotName;
	
};
