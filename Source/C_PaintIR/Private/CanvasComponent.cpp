// Fill out your copyright notice in the Description page of Project Settings.


#include "CanvasComponent.h"

#include "ImageUtils.h"
#include "Engine/Texture2D.h"
#include "TextureUtils.h"
#include "Engine/Texture.h"
#include "CustomShadersDeclarations/Private/APCSDeclaration.h"

#include "RHICommandList.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

// Sets default values for this component's properties
UCanvasComponent::UCanvasComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	CurrentDrawMode = ECanvasDrawMode::Temperature; // 默认绘制模式

	// 创建 SceneCaptureComponent2D
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	if (SceneCapture)
	{
		SceneCapture->SetupAttachment(this);
		// Unreal 引擎会 自动在组件构造、Actor创建并附加到世界时帮你注册这些子组件，你不需要手动注册，也不应该这么做，尤其是在构造函数里。
		// SceneCapture->RegisterComponent();

		// 配置 SceneCapture
		SceneCapture->FOVAngle = 90.0f;
		SceneCapture->ProjectionType = ECameraProjectionMode::Type::Orthographic;
		SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
		SceneCapture->bCaptureEveryFrame = false;
		SceneCapture->bCaptureOnMovement = false;
		
		FVector Location(0.0f, 0.0f, 200.0f);
		FRotator Rotation(0.0f, -90.0f, -90.0f);  // 朝下看
		FVector Scale(1.0f, 1.0f, 1.0f);

		SceneCapture->SetRelativeTransform(FTransform(Rotation, Location, Scale));
		
	}

	TemperatureDrawingComp = CreateDefaultSubobject<UTemperatureDrawingComponent>(TEXT("TemperatureDrawingComp"));
	CustomDrawingComp = CreateDefaultSubobject<UCustomDrawingComponent>(TEXT("CustomDrawingComp"));

	// 加载材质资产
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnwarpMaterialAsset(TEXT("Material'/Game/Materials/M_Unwarp.M_Unwarp'"));
	if (UnwarpMaterialAsset.Succeeded())
	{
		UnwrapMaterial = UMaterialInstanceDynamic::Create(UnwarpMaterialAsset.Object, this);
	}

	// 加载材质资产
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> IRMaterialAsset(TEXT("Material'/Game/Materials/M_IR.M_IR'"));
	if (IRMaterialAsset.Succeeded())
	{
		IRMaterial = UMaterialInstanceDynamic::Create(IRMaterialAsset.Object, this);
	}
}


// Called when the game starts
void UCanvasComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UCanvasComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCanvasComponent::InitializeForMesh(UStaticMeshComponent* MeshComponent)
{
	if (!MeshComponent) return;

	RenderTarget = NewObject<UTextureRenderTarget2D>(this);
	RenderTarget->InitAutoFormat(Settings.TextureLength, Settings.TextureLength);
	RenderTarget->ClearColor = FLinearColor::Black;
	RenderTarget->UpdateResourceImmediate();
	
	SceneCapture->TextureTarget = RenderTarget;
	SceneCapture->OrthoWidth = Settings.OrthoWidth; 

	const FBoxSphereBounds MyBounds = MeshComponent->Bounds;
	const FVector Center = MyBounds.Origin;

	//把包围盒中心位置作为展开位置
	UnwrapMaterial->SetScalarParameterValue(FName("UnwrapScale"), Settings.UnwrapScale);
	UnwrapMaterial->SetVectorParameterValue(FName("UnwrapLocation"), Center);
}

void UCanvasComponent::DrawPoint(const FVector& WorldLocation, float Value, UStaticMeshComponent* MeshComponent)
{
	// 将世界坐标转换为组件的局部坐标
	FVector LocalLocation = GetComponentTransform().InverseTransformPosition(WorldLocation);
	UE_LOG(LogTemp, Log, TEXT("Clicked at local position: %s, and current value is :%f"), *LocalLocation.ToString(), Value);

	DrawnPoints.Add(LocalLocation, Value);

	UStaticMesh* StaticMesh = MeshComponent->GetStaticMesh();
	if (!StaticMesh) return;

	//1. 展UV
	ApplyUnwrapMaterial(MeshComponent);

	//2. 捕获
	SceneCapture->CaptureScene();
	
	// // 获取网格体的包围盒
	// const FBoxSphereBounds MeshBounds = StaticMesh->GetBounds();
	//
	// FStaticMeshRenderData* RenderData = StaticMesh->GetRenderData();
	// if (!RenderData || RenderData->LODResources.Num() == 0) return;
	//
	// const FStaticMeshLODResources& LODResource = RenderData->LODResources[0];
	// const FPositionVertexBuffer& PositionBuffer = LODResource.VertexBuffers.PositionVertexBuffer;
	// const FStaticMeshVertexBuffer& VertexBuffer = LODResource.VertexBuffers.StaticMeshVertexBuffer;
	//
	// // 顶点数量是一致的
	// int32 VertexCount = PositionBuffer.GetNumVertices();
	//
	// // 准备顶点位置和 UV 数据
	// TArray<FVector3f> VertexPositions;
	// TArray<FVector2f> VertexUVs;
	// VertexPositions.SetNum(VertexCount);
	// VertexUVs.SetNum(VertexCount);
	//
	// for (int32 i = 0; i < VertexCount; ++i)
	// {
	// 	VertexPositions[i] = PositionBuffer.VertexPosition(i);
	// 	VertexUVs[i] = VertexBuffer.GetVertexUV(i, 0); // 使用第一个 UV 通道
	// }
	//
	// UploadVertexBufferToGPU(VertexPositions, VertexUVs, MeshBounds);
}

void UCanvasComponent::UploadVertexBufferToGPU(const TArray<FVector3f>& VertexPositions, const TArray<FVector2f>& VertexUVs, const FBoxSphereBounds& MeshBounds)
{
	const uint32 PositionBufferSize = VertexPositions.Num() * sizeof(FVector3f);
	const uint32 UVBufferSize = VertexUVs.Num() * sizeof(FVector2f);

	// 使用共享指针防止数据生命周期问题
	TSharedPtr<TArray<FVector3f>> VertexPositionsCopy = MakeShared<TArray<FVector3f>>(VertexPositions);
	TSharedPtr<TArray<FVector2f>> VertexUVsCopy = MakeShared<TArray<FVector2f>>(VertexUVs);

    ENQUEUE_RENDER_COMMAND(UploadVertexAndUVBuffers)(
        [this, MeshBounds, VertexPositionsCopy, VertexUVsCopy, PositionBufferSize, UVBufferSize](FRHICommandListImmediate& RHICmdList)
        {
            // 创建顶点位置结构化缓冲区
            FRHIResourceCreateInfo PositionCreateInfo(TEXT("VertexPositionBuffer"));
            FBufferRHIRef PositionBuffer = RHICmdList.CreateStructuredBuffer(
                sizeof(FVector3f),
                PositionBufferSize,
                BUF_ShaderResource,
                PositionCreateInfo
            );

            void* PositionBufferData = RHICmdList.LockBuffer(PositionBuffer, 0, PositionBufferSize, RLM_WriteOnly);
            FMemory::Memcpy(PositionBufferData, VertexPositionsCopy->GetData(), PositionBufferSize);
            RHICmdList.UnlockBuffer(PositionBuffer);
            FShaderResourceViewRHIRef PositionBufferSRV = RHICmdList.CreateShaderResourceView(PositionBuffer);

            // 创建 UV 结构化缓冲区
            FRHIResourceCreateInfo UVCreateInfo(TEXT("VertexUVBuffer"));
            FBufferRHIRef UVBuffer = RHICmdList.CreateStructuredBuffer(
                sizeof(FVector2f),
                UVBufferSize,
                BUF_ShaderResource,
                UVCreateInfo
            );

            void* UVBufferData = RHICmdList.LockBuffer(UVBuffer, 0, UVBufferSize, RLM_WriteOnly);
            FMemory::Memcpy(UVBufferData, VertexUVsCopy->GetData(), UVBufferSize);
            RHICmdList.UnlockBuffer(UVBuffer);
            FShaderResourceViewRHIRef UVBufferSRV = RHICmdList.CreateShaderResourceView(UVBuffer);

            // 创建输出纹理
            int32 Width = 1024;
            int32 Height = 1024;
            FRHITextureCreateDesc Desc = FRHITextureCreateDesc::Create2D(TEXT("MyRWTexture"))
                .SetExtent(Width, Height)
                .SetFormat(PF_R8G8B8A8)
                .SetFlags(TexCreate_ShaderResource | TexCreate_UAV | TexCreate_RenderTargetable)
                .SetNumMips(1);

            FTexture2DRHIRef OutputTexture = RHICreateTexture(Desc)->GetTexture2D();
            FUnorderedAccessViewRHIRef OutputTextureUAV = RHICmdList.CreateUnorderedAccessView(OutputTexture);

        	// 创建计算着色器的参数
			FAPCSParameters ShaderParameters;
			ShaderParameters.VertexPositions = PositionBufferSRV;
			ShaderParameters.VertexUVs = UVBufferSRV;
			ShaderParameters.OutputTexture = OutputTextureUAV;
			ShaderParameters.VertexCount = VertexPositionsCopy->Num();
			ShaderParameters.MinBound = FVector3f(0.0f, 0.0f, 0.0f);  // 这里使用适当的包围盒值
			ShaderParameters.MaxBound = FVector3f(1.0f, 1.0f, 1.0f);  // 这里使用适当的包围盒值
        	
        	// 使用获取的包围盒的最小值和最大值
        	// 手动计算包围盒的 Min 和 Max
			FVector3f MinBound = FVector3f(MeshBounds.Origin - MeshBounds.BoxExtent);
			FVector3f MaxBound = FVector3f(MeshBounds.Origin + MeshBounds.BoxExtent);

			ShaderParameters.MinBound = MinBound;
			ShaderParameters.MaxBound = MaxBound;
        	
			ShaderParameters.TextureWidth = Width;
			ShaderParameters.TextureHeight = Height;
        	
			FAPCSManager::Dispatch(RHICmdList, ShaderParameters);

        	FString FilePath = FPaths::ProjectContentDir() + TEXT("Textures/OutputTexture.png");

        	SaveTextureToDisk(OutputTexture, FilePath);
        }
    );
}


void UCanvasComponent::SetDrawMode(ECanvasDrawMode NewMode)
{
	CurrentDrawMode = NewMode;
}

void UCanvasComponent::ApplyUnwrapMaterial(UStaticMeshComponent* MeshComponent)
{
	if (MeshComponent && UnwrapMaterial)
	{
		int32 MaterialCount = MeshComponent->GetNumMaterials();
		for (int32 Index = 0; Index < MaterialCount; ++Index)
		{
			MeshComponent->SetMaterial(Index, UnwrapMaterial);
		}
	}
}

// 待修改，用于将生成的纹理结果换给模型
void UCanvasComponent::SetMeshMaterial(UStaticMeshComponent* MeshComponent)
{
	if (MeshComponent)
	{
		// 生成纹理
		UTexture2D* GeneratedTexture = FTextureUtils::GenerateTexture();

		if (GeneratedTexture)
		{
			if (IRMaterial)
			{
				// 设置生成的纹理到材质的参数槽
				IRMaterial->SetTextureParameterValue(TEXT("TextureParam"), GeneratedTexture);
			
				// 获取网格体的材质数量
				int32 SlotCount = MeshComponent->GetNumMaterials();
	
				// 遍历所有材质槽，应用动态材质
				for (int32 i = 0; i < SlotCount; ++i)
				{
					MeshComponent->SetMaterial(i, IRMaterial);
				}
			}
		}
	}
}

void UCanvasComponent::SaveTextureToDisk(FTexture2DRHIRef OutputTexture, const FString& FilePath)
{
	// 在渲染线程中进行操作时，使用 ENQUEUE_RENDER_COMMAND
	ENQUEUE_RENDER_COMMAND(SaveTextureCommand)(
		[this, OutputTexture, FilePath](FRHICommandListImmediate& RHICmdList)
		{
			// 使用 Enqueue 来确保操作在正确的线程中执行
			AsyncTask(ENamedThreads::GameThread, [this, OutputTexture, FilePath]()
			{
				// 确保 SaveTextureToDisk 在游戏线程中执行
				SaveTextureToDiskOnGameThread(OutputTexture, FilePath);
			});
		}
	);
}

void UCanvasComponent::SaveTextureToDiskOnGameThread(FTexture2DRHIRef OutputTexture, const FString& FilePath)
{
	// 获取纹理的宽高
	int32 Width = OutputTexture->GetSizeX();
	int32 Height = OutputTexture->GetSizeY();

	// 创建一个缓冲区来存储读取的图像数据
	TArray<FColor> OutImageData;
	OutImageData.SetNumUninitialized(Width * Height);  // 根据纹理的尺寸调整大小

	// 读取纹理内容到内存
	ENQUEUE_RENDER_COMMAND(ReadTextureData)(
		[OutputTexture, &OutImageData, Width, Height](FRHICommandListImmediate& RHICmdList)
		{
			// 使用RHI读取纹理数据到缓冲区
			RHICmdList.ReadSurfaceData(OutputTexture, FIntRect(0, 0, Width, Height), OutImageData, FReadSurfaceDataFlags());
		}
	);

	// 等待GPU命令执行完毕，确保图像数据已经读取
	FlushRenderingCommands();

	// 创建一个TArray<uint8>存储PNG格式的图像数据
	TArray<uint8> CompressedData;

	// 使用FImageUtils将图像数据压缩为PNG
	FImageUtils::CompressImageArray(Width, Height, OutImageData, CompressedData);
	
	// 保存压缩的图像数据到文件
	if (FFileHelper::SaveArrayToFile(CompressedData, *FilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("Texture saved to %s"), *FilePath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save texture to %s"), *FilePath);
	}
}