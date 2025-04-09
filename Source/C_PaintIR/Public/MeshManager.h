// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MeshManager.generated.h"

/**
 * 
 */
UCLASS()
class C_PAINTIR_API UMeshManager : public UObject
{
	GENERATED_BODY()

public:
	// 构造函数
	UMeshManager();
	
	void LoadMeshes(const FString& AssetPath);

private:

	// 存储加载的静态网格体
	TArray<UStaticMesh*> StaticMeshes;

	// 示例：在您的类的头文件中
	UPROPERTY()
	TArray<AActor*> Actors;
};
