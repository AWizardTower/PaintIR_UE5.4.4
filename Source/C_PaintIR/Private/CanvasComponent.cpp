// Fill out your copyright notice in the Description page of Project Settings.


#include "CanvasComponent.h"
#include "Engine/Texture2D.h"
#include "TextureUtils.h"
#include "Engine/Texture.h"

// Sets default values for this component's properties
UCanvasComponent::UCanvasComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	CurrentDrawMode = ECanvasDrawMode::Temperature; // 默认绘制模式

	TemperatureDrawingComp = CreateDefaultSubobject<UTemperatureDrawingComponent>(TEXT("TemperatureDrawingComp"));
	CustomDrawingComp = CreateDefaultSubobject<UCustomDrawingComponent>(TEXT("CustomDrawingComp"));

	// 加载材质资产
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialAsset(TEXT("Material'/Game/Materials/M_MyMaterial.M_MyMaterial'"));
	DynamicMaterial = UMaterialInstanceDynamic::Create(MaterialAsset.Object, this);
}


// Called when the game starts
void UCanvasComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UCanvasComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCanvasComponent::DrawPoint(const FVector& WorldLocation, float)
{
	// 将世界坐标转换为组件的局部坐标
	FVector LocalLocation = GetComponentTransform().InverseTransformPosition(WorldLocation);
	// 输出局部坐标
	UE_LOG(LogTemp, Log, TEXT("Clicked at local position: %s"), *LocalLocation.ToString());
}

void UCanvasComponent::SetDrawMode(ECanvasDrawMode NewMode)
{
	CurrentDrawMode = NewMode;
}

// 设置网格体材质的纹理
void UCanvasComponent::SetMeshMaterial(UStaticMeshComponent* MeshComponent)
{
	if (MeshComponent)
	{
		// 生成纹理
		UTexture2D* GeneratedTexture = FTextureUtils::GenerateTexture();

		if (GeneratedTexture)
		{
			if (DynamicMaterial)
			{
				// 设置生成的纹理到材质的参数槽
				DynamicMaterial->SetTextureParameterValue(TEXT("TextureParam"), GeneratedTexture);
			
				// 获取网格体的材质数量
				int32 SlotCount = MeshComponent->GetNumMaterials();
	
				// 遍历所有材质槽，应用动态材质
				for (int32 i = 0; i < SlotCount; ++i)
				{
					MeshComponent->SetMaterial(i, DynamicMaterial);
				}
			}
		}
	}
}