// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalSettingsManager.h"
#include "MeshManager.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class C_PAINTIR_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UMyGameInstance();
	
	virtual void Init() override;

	UFUNCTION(BlueprintCallable, Category = "MeshManager")
	UMeshManager* GetMeshManager() const { return MeshManager; }

	UFUNCTION(BlueprintCallable, Category = "Global Setting")
	UGlobalSettingsManager* GetGlobalSettingsManager() const { return SettingsManager; }
private:
	UPROPERTY()
	UMeshManager* MeshManager;

	// 全局Canvas设置管理器
	UPROPERTY()
	UGlobalSettingsManager* SettingsManager;
};
