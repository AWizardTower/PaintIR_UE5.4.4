// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"
#include "PointVisualizerComponent.generated.h"

USTRUCT()
struct FKeyPointVisual
{
	GENERATED_BODY()

	// 关键点的世界坐标
	UPROPERTY()
	FVector Position;

	// 当前值（如温度）
	UPROPERTY()
	float Value;

	// 显示球体
	UPROPERTY()
	UStaticMeshComponent* SphereComponent = nullptr;

	// 悬浮UI（显示值、修改、删除按钮）
	UPROPERTY()
	UWidgetComponent* WidgetComponent = nullptr;
};

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
	// 应该从画布组件中获取
	UPROPERTY()
	TMap<FVector, float> KeyPoints;

	UPROPERTY()
	TArray<FKeyPointVisual> KeyPointVisuals;

	// 是一个 类的类型（不是实例！是“模板”）
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> KeyPointWidgetClass; // 指向你的蓝图Widget类

	UPROPERTY(EditAnywhere)
	UStaticMesh* SphereMesh;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* SphereMaterial;
		
};
