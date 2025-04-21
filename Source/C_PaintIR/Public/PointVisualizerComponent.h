// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PointVisualizerComponent.generated.h"


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
	void RemoveKeyPoint(const FVector& WorldPosition);

	// 修改关键点
	void ModifyKeyPoint(const FVector& WorldPosition, float NewValue);

	// 可视化重建（每次 Add/Remove 后都调用）
	void RefreshVisuals();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	UPROPERTY()
	TMap<FVector, float> KeyPoints;

	UPROPERTY()
	TArray<UStaticMeshComponent*> VisualizedPoints;

	UPROPERTY()
	UStaticMesh* SphereMesh;

	UPROPERTY()
	UMaterialInterface* LabelMaterialBase;
		
};
