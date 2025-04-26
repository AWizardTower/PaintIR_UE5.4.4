// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PointComponent.h"
#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"
#include "PointVisualizerComponent.generated.h"

class UCanvasComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class C_PAINTIR_API UPointVisualizerComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPointVisualizerComponent();
	// 添加关键点
	void AddKeyPoint(const FVector& WorldPosition, float Value);

	// 删除关键点
	void RemoveKeyPoint(UPointComponent* PointComponent);

	// 修改关键点
	void ModifyKeyPointValue(const FVector& WorldPosition, float NewValue);

	// 更新所有关键点的可视化
	void UpdateAllVisualizers(const TMap<FVector, float>& NewPoints);

	UPROPERTY()
	UCanvasComponent* OwningCanvas;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	// 存储所有的关键点组件
	UPROPERTY()
	TArray<UPointComponent*> PointComponents;
};
