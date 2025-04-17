// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameModeBase.h"
#include "MyGameInstance.h" // 引入 GameInstance 头文件
#include "MyPlayerController.h"


AMyGameModeBase::AMyGameModeBase()
{
	PlayerControllerClass = AMyPlayerController::StaticClass(); // 设置自定义 PlayerController
}

void AMyGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// 获取当前世界的 GameInstance
	UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
	if (GameInstance)
	{
		// 获取 MeshManager 实例
		UMeshManager* MeshManager = GameInstance->GetMeshManager();
		if (MeshManager)
		{
			MeshManager->LoadMeshes(TEXT("/Game/650FlightScene/作战实体模型/车辆"));

			//并不专门存储一个MyPlayerController的引用
			AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
			
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("MeshManager 实例为空"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GameInstance 获取失败"));
	}
}
