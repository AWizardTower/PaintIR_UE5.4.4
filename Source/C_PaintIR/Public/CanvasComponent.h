// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TemperatureDrawingComponent.h"
#include "CustomDrawingComponent.h"
#include "CanvasComponent.generated.h"  // 这一句必须在最后一行


UENUM(BlueprintType)
enum class ECanvasDrawMode : uint8
{
	Temperature UMETA(DisplayName = "Temperature"),
	Color UMETA(DisplayName = "Color"),
	Infrared UMETA(DisplayName = "Infrared"),
	Custom UMETA(DisplayName = "Custom")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class C_PAINTIR_API UCanvasComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCanvasComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void DrawPoint(const FVector& WorldLocation, float Value, UStaticMeshComponent* MeshComponent);

	// 设置当前绘制模式
	void SetDrawMode(ECanvasDrawMode NewMode);

	// 设置网格材质的纹理
	void SetMeshMaterial(UStaticMeshComponent* MeshComponent);

	void UploadVertexBufferToGPU(const TArray<FVector3f>& VertexPositions, const TArray<FVector2f>& VertexUVs, const FBoxSphereBounds& Bounds);

	void SaveTextureToDisk(FTexture2DRHIRef OutputTexture, const FString& FilePath);

private:
	ECanvasDrawMode CurrentDrawMode;
	TMap<FVector, float> DrawnPoints; // 存储已绘制点的位置和对应的值

	UPROPERTY()
	UTemperatureDrawingComponent* TemperatureDrawingComp;

	UPROPERTY()
	UCustomDrawingComponent* CustomDrawingComp;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;

	void SaveTextureToDiskOnGameThread(FTexture2DRHIRef OutputTexture, const FString& FilePath);
	
};
