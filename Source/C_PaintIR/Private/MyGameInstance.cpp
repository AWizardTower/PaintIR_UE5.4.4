// Fill out your copyright notice in the Description page of Project Settings.

#include "C_PaintIR/Public/MyGameInstance.h"

UMyGameInstance::UMyGameInstance()
{
	// 在构造函数中初始化 MeshManager 程序会崩溃
	// MeshManager = NewObject<UMeshManager>(this, UMeshManager::StaticClass());
}

void UMyGameInstance::Init()
{
	Super::Init();
	MeshManager = NewObject<UMeshManager>(this, UMeshManager::StaticClass());
	// 其他初始化逻辑

	// 调用 MeshManager 的方法，例如加载网格
	//if (MeshManager)
	//{
	//	MeshManager->LoadMeshes(TEXT("/Game/650FlightScene/作战实体模型/车辆"));
	//}

	// 在屏幕上显示调试消息
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("UMyGameInstance::Init() called"));
	}
}
