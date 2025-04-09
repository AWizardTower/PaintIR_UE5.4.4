// Fill out your copyright notice in the Description page of Project Settings.

#include "C_PaintIR/Public/MeshManager.h"
#include "Engine/ObjectLibrary.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMeshActor.h"

UMeshManager::UMeshManager()
{
	
}

void UMeshManager::LoadMeshes(const FString& AssetPath)
{
	// 创建对象库
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), false, true);
	
	ObjectLibrary->LoadAssetDataFromPath(AssetPath);
	ObjectLibrary->LoadAssetsFromAssetData();

	// 获取加载的资产数据
	TArray<FAssetData> AssetDataList;
	ObjectLibrary->GetAssetDataList(AssetDataList);
	
	// 定义初始位置和间距
	FVector StartLocation = FVector(0.f, 1000.f, 0.f);
	float OffsetX = 2000.f; // 每个 Actor 在 X 轴上的间距

	int32 Index = 0;
	for (const FAssetData& AssetData : AssetDataList)
	{
		UStaticMesh* StaticMesh = Cast<UStaticMesh>(AssetData.GetAsset());
		if (StaticMesh)
		{
			StaticMeshes.Add(StaticMesh);

			// 输出静态网格体的名称
			UE_LOG(LogTemp, Warning, TEXT("Loaded Static Mesh: %s"), *StaticMesh->GetName());

			UWorld* World = GetWorld();
			if (World)
			{
				// 计算当前 Actor 的位置
				FVector ActorLocation = StartLocation + FVector(Index * OffsetX, 0.f, 0.f);

				// 在世界中生成一个新的 AStaticMeshActor
				AStaticMeshActor* NewActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), ActorLocation, FRotator::ZeroRotator);
				if (NewActor)
				{
					// 设置 Actor 的静态网格体
					NewActor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);

					// 设置 Actor 的标签为资产名称
					FString AssetName = AssetData.AssetName.ToString();
					NewActor->SetActorLabel(AssetName);
					
					// 可选：设置 Actor 的缩放
					NewActor->SetActorScale3D(FVector(1.f, 1.f, 1.f));

					// 将新创建的 Actor 添加到数组中
					Actors.Add(NewActor);

					// 设置 Actor 所属的文件夹路径
					NewActor->SetFolderPath(TEXT("Mesh"));

					// 更新索引以便下一个 Actor 的位置
					Index++;
				}
			}
		}
	}
	
}


