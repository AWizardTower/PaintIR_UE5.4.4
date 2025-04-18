// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TemperatureDrawingComponent.h"
#include "CustomDrawingComponent.h"
#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "CanvasComponent.generated.h"  // 这一句必须在最后一行

//创建一个专门的参数结构 FCanvasRenderSettings，在多个类中作为成员使用或传递
USTRUCT(BlueprintType)
struct FCanvasComponentSettings
{
	GENERATED_BODY()

	// RenderTarget 尺寸
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canvas Settings")
	int32 TextureLength = 2048;

	// 展开尺寸 应该用不着严丝合缝
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canvas Settings")
	int32 UnwrapScale = 2000;

	// 正交相机捕获范围
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canvas Settings")
	int32 OrthoWidth = 2048;

	// SceneCapture 相对于模型的偏移位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canvas Settings")
	FVector CaptureOffset = FVector(0, 0, 200);

	// 捕捉方向（默认朝下看）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canvas Settings")
	FRotator CaptureRotation = FRotator(-90, 0, 0);

	// 是否只捕捉一次
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canvas Settings")
	bool bCaptureOnce = true;
};

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
	void InitializeForMesh(UStaticMeshComponent* MeshComponent);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void DrawPoint(const FVector& WorldLocation, float Value, UStaticMeshComponent* MeshComponent);

	// 设置当前绘制模式
	void SetDrawMode(ECanvasDrawMode NewMode);

	// 设置网格材质的纹理
	void SetMeshMaterial(UStaticMeshComponent* MeshComponent);

	void UploadVertexBufferToGPU(const TArray<FVector3f>& VertexPositions, const TArray<FVector2f>& VertexUVs, const FBoxSphereBounds& Bounds);

	void SaveTextureToDisk(FTexture2DRHIRef OutputTexture, const FString& FilePath);

	void ApplyUnwrapMaterial(UStaticMeshComponent* MeshComponent);

	void CopyRenderTargetRHI(UTextureRenderTarget2D* SourceRT, UTextureRenderTarget2D* DestRT);
private:
	ECanvasDrawMode CurrentDrawMode;
	TMap<FVector, float> DrawnPoints; // 存储已绘制点的位置和对应的值

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

	UPROPERTY()
	FCanvasComponentSettings Settings;

	// CanvasComponent.h
	UPROPERTY()
	UTextureRenderTarget2D* RTDisplayIR = nullptr;
};
