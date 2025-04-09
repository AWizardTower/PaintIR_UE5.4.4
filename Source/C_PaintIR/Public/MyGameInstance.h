// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	
	UMeshManager* GetMeshManager() const { return MeshManager; }
	
private:
	UPROPERTY()
	UMeshManager* MeshManager;
	
};
