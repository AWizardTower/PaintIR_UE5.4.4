#pragma once

#include "CoreMinimal.h"
#include "KeyPointData.generated.h"

USTRUCT(BlueprintType)
struct FKeyPointData
{
	GENERATED_BODY()

	// 包含关键点的 Map
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeyPointData")
	TMap<FVector, float> Points;

	FKeyPointData() {}

	FKeyPointData(const TMap<FVector, float>& InPoints)
		: Points(InPoints) {}
	
};