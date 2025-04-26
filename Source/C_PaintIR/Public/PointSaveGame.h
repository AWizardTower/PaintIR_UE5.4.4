// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "KeyPointData.h"
#include "PointSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FModelDrawData
{
	GENERATED_BODY()

	UPROPERTY()
	FName ModelName;

	UPROPERTY()
	FKeyPointData DrawnPoints;
};

UCLASS()
class C_PAINTIR_API UPointSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FModelDrawData> AllModelDrawData;
};