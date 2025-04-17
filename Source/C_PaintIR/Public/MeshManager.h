// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MeshManager.generated.h"

// 声明委托类型
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMeshLoadComplete);

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

	// 委托：通知外部加载完成
	UPROPERTY(BlueprintAssignable, Category = "MeshManager")
	FOnMeshLoadComplete OnMeshLoadComplete;
	
	void LoadMeshes(const FString& AssetPath);

	UFUNCTION(BlueprintCallable, Category = "MeshManager")
	int32 GetMeshCount() const;
	
	// 获取当前模型
	UFUNCTION(BlueprintCallable, Category = "MeshManager")
	AMyStaticMeshActor* GetCurrentActor() const;

	UFUNCTION(BlueprintCallable, Category = "MeshManager")
	AMyStaticMeshActor* NextActor();

	UFUNCTION(BlueprintCallable, Category = "MeshManager")
	AMyStaticMeshActor* PreviousActor();

	// 按名称或索引查找
	UFUNCTION(BlueprintCallable, Category = "MeshManager")
	AMyStaticMeshActor* FindActorByName(const FString& Name) const;

	UFUNCTION(BlueprintCallable, Category = "MeshManager")
	AMyStaticMeshActor* FindActorByIndex(int32 Index);

	UFUNCTION(BlueprintCallable)
	int32 GetCurrentMeshIndex() const;

	UFUNCTION(BlueprintCallable, Category = "MeshManager")
	TArray<FString> GetActorIndexNameList() const;
private:

	// 存储加载的静态网格体
	TArray<UStaticMesh*> StaticMeshes;
	
	UPROPERTY()
	int32 CurrentActorIndex = 0;

	// 示例：在您的类的头文件中
	UPROPERTY()
	TArray<AMyStaticMeshActor*> Actors;
};
