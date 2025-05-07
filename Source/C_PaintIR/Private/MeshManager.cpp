// Fill out your copyright notice in the Description page of Project Settings.

#include "C_PaintIR/Public/MeshManager.h"

#include "CanvasSaveManager.h"
#include "KeyPointData.h"
#include "MyStaticMeshActor.h"
#include "Engine/ObjectLibrary.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "PhysicsEngine/BodySetup.h"
#include "Misc/Paths.h" // 注意要加这个，处理路径

UMeshManager::UMeshManager()
{
	// 加载材质资产
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> IRMaterialAsset(TEXT("Material'/Game/Materials/M_BaseColor.M_BaseColor'"));
	if (IRMaterialAsset.Succeeded())
	{
		BaseMaterial = UMaterialInstanceDynamic::Create(IRMaterialAsset.Object, this);
	}
}

// 显示当前模型，隐藏其他模型
void UMeshManager::ShowCurrentActorOnly()
{
	for (AMyStaticMeshActor* Actor : Actors)
	{
		if (Actor)
		{
			// 如果是当前模型，显示；否则，隐藏
			Actor->SetActorHiddenInGame(Actor != GetCurrentActor());
		}
	}
}

// 显示所有模型
void UMeshManager::ShowAllActors()
{
	for (AMyStaticMeshActor* Actor : Actors)
	{
		if (Actor)
		{
			// 显示所有模型
			Actor->SetActorHiddenInGame(false);
		}
	}
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
	float OffsetX = 5000.f; // 每个 Actor 在 X 轴上的间距

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

				FActorSpawnParameters SpawnParams;
				SpawnParams.Name = FName(*AssetData.AssetName.ToString());
				AMyStaticMeshActor* NewActor = World->SpawnActor<AMyStaticMeshActor>(AMyStaticMeshActor::StaticClass(), ActorLocation, FRotator::ZeroRotator, SpawnParams);
				
				// 在世界中生成一个新的 AStaticMeshActor
				//AMyStaticMeshActor* NewActor = World->SpawnActor<AMyStaticMeshActor>(AMyStaticMeshActor::StaticClass(), ActorLocation, FRotator::ZeroRotator);
				if (NewActor)
				{
					// 设置 Actor 的静态网格体
					NewActor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);

					// 启用精确碰撞
					// 设置碰撞复杂性为复杂作为简单
					UStaticMeshComponent* MeshComp = NewActor->GetStaticMeshComponent();

					if (StaticMesh && StaticMesh->GetBodySetup())
					{
						StaticMesh->GetBodySetup()->CollisionTraceFlag = CTF_UseComplexAsSimple;

						// 确保更新物理状态（MeshComp 必须先设置了 StaticMesh）
						MeshComp->RecreatePhysicsState();
					}
					// NewActor->GetStaticMeshComponent()->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName); // 可根据需求调整碰撞配置
					// NewActor->GetStaticMeshComponent()->BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					// NewActor->GetStaticMeshComponent()->SetCollisionObjectType(ECC_WorldStatic);
					// NewActor->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Block);
					
					// 设置 Actor 的标签为资产名称
					FString AssetName = AssetData.AssetName.ToString();
					NewActor->SetActorLabel(AssetName);
					
					// 可选：设置 Actor 的缩放
					NewActor->SetActorScale3D(FVector(1.f, 1.f, 1.f));

					// 根据模型初始化组件参数
					NewActor->CanvasComponent->InitializeForMesh();

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
	ShowCurrentActorOnly();
	SetMaterialForAllActors(BaseMaterial);
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
	ShowCurrentActorOnly();
	return Cast<AMyStaticMeshActor>(Actors[CurrentActorIndex]);
}

AMyStaticMeshActor* UMeshManager::PreviousActor()
{
	if (Actors.Num() == 0) return nullptr;

	CurrentActorIndex = (CurrentActorIndex - 1 + Actors.Num()) % Actors.Num();
	ShowCurrentActorOnly();
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
		ShowCurrentActorOnly();
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

void UMeshManager::CollectAllCanvasData()
{
	TMap<FName, FKeyPointData> AllDrawnData;
	
	AllDrawnData.Empty();

	for (AMyStaticMeshActor* Actor : Actors)
	{
		if (!Actor) continue;

		UCanvasComponent* CanvasComp = Actor->FindComponentByClass<UCanvasComponent>();
		if (CanvasComp)
		{
			FName ModelName = Actor->GetFName(); // 或者用 CanvasComp->GetAssociatedModelName() 如果你自定义了
			FKeyPointData PointData = FKeyPointData(CanvasComp->DrawnPoints);
			AllDrawnData.Add(ModelName, PointData.Points);
			
			UE_LOG(LogTemp, Warning, TEXT("保存模型 [%s] 的关键点数据:"), *ModelName.ToString());
			for (const TPair<FVector, float>& Pair : PointData.Points)
			{
				UE_LOG(LogTemp, Warning, TEXT("位置: %s，值: %.2f"), *Pair.Key.ToString(), Pair.Value);
			}
		}
	}
	// 示例：保存
	UCanvasSaveManager::SaveAllCanvasData(AllDrawnData);

	if (GEngine)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("绘制数据保存成功！"));
	}
}

void UMeshManager::RestoreAllCanvasData()
{
	TMap<FName, FKeyPointData> LoadedData = UCanvasSaveManager::LoadAllCanvasData();
	
	// 输出全部加载的数据
	for (const TPair<FName, FKeyPointData>& ModelPair : LoadedData)
	{
		const FName& ModelName = ModelPair.Key;
		const FKeyPointData& KeyPointData = ModelPair.Value;

		UE_LOG(LogTemp, Warning, TEXT("模型 [%s] 的关键点数据:"), *ModelName.ToString());

		for (const TPair<FVector, float>& PointPair : KeyPointData.Points)
		{
			UE_LOG(LogTemp, Warning, TEXT("    位置: %s，值: %.2f"), *PointPair.Key.ToString(), PointPair.Value);
		}
	}
	
	for (AMyStaticMeshActor* Actor : Actors)
	{
		if (!Actor) continue;
		UCanvasComponent* CanvasComp = Actor->FindComponentByClass<UCanvasComponent>();
		if (CanvasComp)
		{
			FName ModelName = Actor->GetFName();
			
			UE_LOG(LogTemp, Warning, TEXT("模型名称:%s"), *ModelName.ToString());
			
			if (LoadedData.Contains(ModelName))
			{
				CanvasComp->LoadFromKeyPointData( LoadedData[ModelName]);  // 你需要自己实现这个方法

				UE_LOG(LogTemp, Warning, TEXT("加载模型 [%s] 的关键点数据:"), *ModelName.ToString());
				for (const TPair<FVector, float>& Pair : LoadedData[ModelName].Points)
				{
					UE_LOG(LogTemp, Warning, TEXT("位置: %s，值: %.2f"), *Pair.Key.ToString(), Pair.Value);
				}
			}
		}
	}

	if (GEngine)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("绘制数据加载完成！"));
	}
}

FString UMeshManager::ExportCurrentCanvasTexture()
{
	FString DirectoryPath = FPaths::ProjectContentDir() + TEXT("Textures");
	AMyStaticMeshActor* CurrentActor = GetCurrentActor();

	if (!CurrentActor)
	{
		return TEXT("导出失败：未找到当前模型！");
	}

	UCanvasComponent* CanvasComp = CurrentActor->FindComponentByClass<UCanvasComponent>();
	if (!CanvasComp)
	{
		return FString::Printf(TEXT("导出失败：模型 %s 上未找到 CanvasComponent！"), *CurrentActor->GetName());
	}

	FString FileName = FString::Printf(TEXT("%s/%s.png"), *DirectoryPath, *CurrentActor->GetMeshName());
	bool bSuccess = CanvasComp->ExportTextureToDisk(FileName);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("导出当前纹理：%s"), *FileName);
		return FString::Printf(TEXT("保存成功：%s"), *FileName);
	}
	else
	{
		return FString::Printf(TEXT("导出失败：无法保存纹理到 %s"), *FileName);
	}
}

void UMeshManager::ExportAllCanvasTextures()
{
	FString DirectoryPath = FPaths::ProjectContentDir() + TEXT("Textures");
	for (AMyStaticMeshActor* Actor : Actors)
	{
		if (Actor)
		{
			UCanvasComponent* CanvasComp = Actor->FindComponentByClass<UCanvasComponent>();
			if (CanvasComp)
			{
				FString FileName = FString::Printf(TEXT("%s/%s.png"), *DirectoryPath, *Actor->GetMeshName());
				CanvasComp->ExportTextureToDisk(FileName);

				UE_LOG(LogTemp, Log, TEXT("导出纹理：%s"), *FileName);
				
			}
		}
	}

	if (GEngine)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("保存所有纹理！"));
	}
}

void UMeshManager::SetMaterialForAllActors(UMaterialInterface* NewMaterial)
{

	if (!NewMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetMaterialForAllActors: NewMaterial is null."));
		return;
	}

	for (AMyStaticMeshActor* Actor : Actors)
	{
		if (Actor && Actor->GetStaticMeshComponent())
		{
			int32 MaterialCount = Actor->GetStaticMeshComponent()->GetNumMaterials();
			for (int32 i = 0; i < MaterialCount; ++i)
			{
				Actor->GetStaticMeshComponent()->SetMaterial(i, NewMaterial);
			}
		}
	}
}