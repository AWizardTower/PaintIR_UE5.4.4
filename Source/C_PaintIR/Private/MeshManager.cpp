// Fill out your copyright notice in the Description page of Project Settings.

#include "C_PaintIR/Public/MeshManager.h"
#include "MyStaticMeshActor.h"
#include "Engine/ObjectLibrary.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"


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
				AMyStaticMeshActor* NewActor = World->SpawnActor<AMyStaticMeshActor>(AMyStaticMeshActor::StaticClass(), ActorLocation, FRotator::ZeroRotator);
				if (NewActor)
				{
					// 设置 Actor 的静态网格体
					NewActor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);

					// 设置 Actor 的标签为资产名称
					FString AssetName = AssetData.AssetName.ToString();
					NewActor->SetActorLabel(AssetName);
					
					// 可选：设置 Actor 的缩放
					NewActor->SetActorScale3D(FVector(1.f, 1.f, 1.f));

					// 根据模型初始化组件参数
					NewActor->CanvasComponent->InitializeForMesh(NewActor->GetStaticMeshComponent());

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

int32 UMeshManager::GetMeshCount() const
{
	return StaticMeshes.Num();
}

AMyStaticMeshActor* UMeshManager::GetCurrentActor() const
{
	if (Actors.IsValidIndex(CurrentActorIndex))
	{
		return Cast<AMyStaticMeshActor>(Actors[CurrentActorIndex]);
	}
	return nullptr;
}

AMyStaticMeshActor* UMeshManager::NextActor()
{
	if (Actors.Num() == 0) return nullptr;

	CurrentActorIndex = (CurrentActorIndex + 1) % Actors.Num();
	return Cast<AMyStaticMeshActor>(Actors[CurrentActorIndex]);
}

AMyStaticMeshActor* UMeshManager::PreviousActor()
{
	if (Actors.Num() == 0) return nullptr;

	CurrentActorIndex = (CurrentActorIndex - 1 + Actors.Num()) % Actors.Num();
	return Cast<AMyStaticMeshActor>(Actors[CurrentActorIndex]);
}

AMyStaticMeshActor* UMeshManager::FindActorByName(const FString& Name) const
{
	for (AActor* Actor : Actors)
	{
		if (Actor && Actor->GetActorLabel() == Name)
		{
			return Cast<AMyStaticMeshActor>(Actor);
		}
	}
	return nullptr;
}

AMyStaticMeshActor* UMeshManager::FindActorByIndex(int32 Index) 
{
	if (Actors.IsValidIndex(Index))
	{
		//同样改变了当前actor
		CurrentActorIndex = Index;
		return Actors[Index];
	}
	return nullptr;
}

int32 UMeshManager::GetCurrentMeshIndex() const
{
	return CurrentActorIndex + 1;
}

TArray<FString> UMeshManager::GetActorIndexNameList() const
{
	TArray<FString> DisplayList;
	for (int32 i = 0; i < Actors.Num(); ++i)
	{
		if (Actors[i])
		{
			FString Entry = FString::Printf(TEXT("%d. %s"), i+1, *Actors[i]->GetMeshName() );
			DisplayList.Add(Entry);
		}
	}
	return DisplayList;
}
