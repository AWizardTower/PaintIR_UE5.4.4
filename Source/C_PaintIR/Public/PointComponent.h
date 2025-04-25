// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"
#include "PointComponent.generated.h"

class UPointVisualizerComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPointValueChanged, float, NewValue);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class C_PAINTIR_API UPointComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPointComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	UPROPERTY(VisibleAnywhere, Category = "Point")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Point")
	UWidgetComponent* WidgetComponent;

public:
	void Cleanup();
	
	UPROPERTY(BlueprintReadWrite, Category = "Point")
	float Value;

	FVector Position;
	
	UPROPERTY()
	UPointVisualizerComponent* OwningVisualizer;
	
	UPROPERTY(BlueprintAssignable)
	FOnPointValueChanged OnValueChanged;
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 是一个 类的类型（不是实例！是“模板”）
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> KeyPointWidgetClass; // 指向你的蓝图Widget类

	// 初始化
    void SetValue(float NewValue);
	
    void SetPosition(const FVector& NewPosition);

	// 更新
	UFUNCTION(BlueprintCallable)
	void UpdateValue(float NewValue);

	UFUNCTION(BlueprintCallable)
	void RemovePoint();

	void InitialUI();

	// 更新关键点可视化（更新网格和UMG）
	void UpdateVisualizer();
};
