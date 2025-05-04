// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TemperatureDrawingComponent.h"
#include "CustomDrawingComponent.h"
#include "GlobalSettingsManager.h"
#include "KeyPointData.h"
#include "PointVisualizerComponent.h"
#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "CanvasComponent.generated.h"  // 这一句必须在最后一行

USTRUCT()
struct FBrushStroke
{
	GENERATED_BODY()

	FVector WorldPosition;       // 笔刷中心
	float Radius;                // 大小
	FLinearColor Color;          // 颜色
	float Hardness;              // 可选：边缘柔和度
	float Strength;              // 叠加强度
};

//创建一个专门的参数结构 FCanvasRenderSettings，在多个类中作为成员使用或传递
// USTRUCT(BlueprintType)
// struct FCanvasComponentSettings
// {
// 	GENERATED_BODY()
//
// 	// RenderTarget 尺寸
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canvas Settings")
// 	int32 TextureLength = 256;
//
// 	// 展开尺寸 应该用不着严丝合缝 还真不行，不把纹理贴满就不满足UV映射
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canvas Settings")
// 	int32 UnwrapScale = 256;
//
// 	// 正交相机捕获范围
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canvas Settings")
// 	int32 OrthoWidth = 256;
//
// 	// 当前属性值最小范围
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canvas Settings")
// 	int32 minVal = 0;
//
// 	// 当前属性值最大范围
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canvas Settings")
// 	int32 maxVal = 100;
// 	
// };

UENUM(BlueprintType)
enum class ECanvasDrawMode : uint8
{
	Temperature UMETA(DisplayName = "Temperature"),
	Color UMETA(DisplayName = "Color"),
	Infrared UMETA(DisplayName = "Infrared"),
	Custom UMETA(DisplayName = "Custom")
};

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
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
	void InitializeForMesh();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void DrawPoint(const FVector& WorldLocation, float Value);

	// 设置当前绘制模式
	void SetDrawMode(ECanvasDrawMode NewMode);

	// 设置网格材质的纹理
	void SetMeshMaterial();

	void UploadVertexBufferToGPU(const TArray<FVector3f>& VertexPositions, const TArray<FVector2f>& VertexUVs, const FBoxSphereBounds& Bounds);

	void SaveTextureToDisk(FTexture2DRHIRef OutputTexture, const FString& FilePath);

	void CopyRenderTargetRHI(UTextureRenderTarget2D* SourceRT, UTextureRenderTarget2D* DestRT);

	void UploadKeyPointsToGPU(const TArray<FVector4f>& KeyPoints);

	void ApplyTextureToMaterial(UTexture2D* GeneratedTexture);

	UFUNCTION()
	void ModifyPointValue(const FVector& WorldLocation, float NewValue);

	UFUNCTION()
	void RemovePoint(const FVector& WorldLocation);

	void LoadFromKeyPointData(const FKeyPointData& Data);

	void GenerateTextureFromDrawnPoints();

	TMap<FVector, float> DrawnPoints; // 存储已绘制点的位置和对应的值

	bool ExportTextureToDisk(const FString& FilePath);

	UPROPERTY(BlueprintReadWrite, Category = "Generated IR Texture")
	UTexture2D* GeneratedIRTexture = nullptr;

	UPROPERTY()
	UStaticMeshComponent* MeshComponent;
	
	void ApplyTextureSize();

private:
	UFUNCTION()
	void OnGlobalSettingsChanged();
	
	UPROPERTY()
	UGlobalSettingsManager* GlobalSettingsManager;  // 保存对设置管理器的引用
	//换上展开材质
	TArray<UMaterialInterface*> ApplyUnwrapMaterial();
	//展开捕获换回的整个流程
	void CaptureWithUnwrapAndRestore();

	ECanvasDrawMode CurrentDrawMode;

	UPROPERTY()
	UTemperatureDrawingComponent* TemperatureDrawingComp;

	UPROPERTY()
	UCustomDrawingComponent* CustomDrawingComp;

	UPROPERTY()
	UMaterialInstanceDynamic* UnwrapMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* IRMaterial;

	void SaveTextureToDiskOnGameThread(FTexture2DRHIRef OutputTexture, const FString& FilePath);

	UPROPERTY()
	USceneCaptureComponent2D* SceneCapture;
	
	UPROPERTY()
	UTextureRenderTarget2D* RenderTarget = nullptr;

	// 用于指定默认的 RenderTarget 资源
	UPROPERTY()
	UTextureRenderTarget2D* DefaultRenderTarget = nullptr;

	//UPROPERTY()
	//FCanvasComponentSettings Settings;

	// CanvasComponent.h
	UPROPERTY()
	UTextureRenderTarget2D* RTDisplayIR = nullptr;

	UPROPERTY()
	UStaticMeshComponent* BackgroundPlane;
	
	// CanvasComponent.h 中加：
	UPROPERTY()
	UPointVisualizerComponent* KeyPointVisualizer;

	UPROPERTY()
	TArray<FBrushStroke> BrushHistory;
};
