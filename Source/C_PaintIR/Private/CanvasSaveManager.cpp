// Fill out your copyright notice in the Description page of Project Settings.


#include "CanvasSaveManager.h"

#include "PointSaveGame.h"
#include "Kismet/GameplayStatics.h"

FString UCanvasSaveManager::SlotName = TEXT("CanvasSaveSlot");

void UCanvasSaveManager::SaveAllCanvasData(const TMap<FName, FKeyPointData>& AllDrawnPoints)
{
	UPointSaveGame* SaveGameInstance = Cast<UPointSaveGame>(UGameplayStatics::CreateSaveGameObject(UPointSaveGame::StaticClass()));
	SaveGameInstance->AllModelDrawData.Empty();

	for (const auto& Pair : AllDrawnPoints)
	{
		FModelDrawData ModelData;
		ModelData.ModelName = Pair.Key;
		ModelData.DrawnPoints = Pair.Value;
		SaveGameInstance->AllModelDrawData.Add(ModelData);  // ← 这行漏了！！！
	}
	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, 0);
}

TMap<FName, FKeyPointData> UCanvasSaveManager::LoadAllCanvasData()
{
	TMap<FName, FKeyPointData> Result;

	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		UPointSaveGame* Loaded = Cast<UPointSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
		if (Loaded)
		{
			for (const FModelDrawData& ModelData : Loaded->AllModelDrawData)
			{
				Result.Add(ModelData.ModelName, ModelData.DrawnPoints);
			}
		}
	}
	return Result;
}