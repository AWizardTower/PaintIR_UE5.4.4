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

		// 可视化 SceneCapture 相机
		SceneCapture->bVisualizeComponent = true;
		SceneCapture->ShowFlags.SetCameraFrustums(true);
		
		// 配置 SceneCapture
		SceneCapture->FOVAngle = 90.0f;
		SceneCapture->ProjectionType = ECameraProjectionMode::Type::Orthographic;
		SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
		SceneCapture->bCaptureEveryFrame = false;
		SceneCapture->bCaptureOnMovement = false;
		
		FVector Location(0.0f, 0.0f, 1000.0f);
		// 因为是RelativeTransform，居然这才是向下看
		// 笑死了，摄像机还必须把纹理的朝向拍对
		FRotator Rotation(-90.0f, 0.0f, -90.0f);  
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
	
	// 构造函数中
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> RTAsset(TEXT("TextureRenderTarget2D'/Game/UI/RT_IRTexturePre.RT_IRTexturePre'"));
	if (RTAsset.Succeeded())
	{
		RTDisplayIR = RTAsset.Object;
	}

	// 创建一个黑色背景平面
	BackgroundPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackgroundPlane"));
	if (BackgroundPlane)
	{
		BackgroundPlane->SetupAttachment(this);

		// 加载基础平面模型
		static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
		if (PlaneMesh.Succeeded())
		{
			BackgroundPlane->SetStaticMesh(PlaneMesh.Object);
		}

		// 设置相对位置（作为 SceneCapture 的拍摄背景）
		BackgroundPlane->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
		BackgroundPlane->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
		BackgroundPlane->SetRelativeScale3D(FVector(10.0f, 10.0f, 1.0f)); // 适当放大

		// 创建并设置黑色材质
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(LoadObject<UMaterial>(nullptr, TEXT("Material'/Game/Materials/M_Background.M_Background'")), this);
		if (Mat)
		{
			BackgroundPlane->SetMaterial(0, Mat);
		}

		// 可选：关闭碰撞，关闭阴影
		BackgroundPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BackgroundPlane->bCastDynamicShadow = false;
		BackgroundPlane->bReceivesDecals = false;
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
	UE_LOG(LogTemp, Log, TEXT("初始化了组件"));
	RenderTarget = NewObject<UTextureRenderTarget2D>(this);
	RenderTarget->InitAutoFormat(Settings.TextureLength, Settings.TextureLength);
	RenderTarget->ClearColor = FLinearColor::Black;
	RenderTarget->UpdateResourceImmediate();
	
	SceneCapture->TextureTarget = RTDisplayIR;
	SceneCapture->OrthoWidth = Settings.OrthoWidth; 

	const FBoxSphereBounds MyBounds = MeshComponent->Bounds;
	// 获取物体包围盒的大小
	FVector BoxExtent = MyBounds.BoxExtent*2;
	// 获取 MeshComponent 的世界位置和包围盒信息
	const FVector ObjectLocation = MeshComponent->GetComponentLocation();
	
	const FVector Center = MyBounds.Origin;
	// 获取 CanvasComponent 根组件的世界位置
	FVector UnwrapLocation = GetComponentLocation()+FVector(0.0f, 0.0f, 2.0f);
	// UE_LOG(LogTemp, Log, TEXT("展开位置设置为: %s"), *CanvasLocation.ToString());
	
	// 把包围盒中心位置作为展开位置不行，和相机捕捉中心对不上
	// 注意名称要对，没有该名称的参数也不会报错，较难排除错误
	UnwrapMaterial->SetScalarParameterValue(FName("UnwrapScale"), Settings.UnwrapScale);
	UnwrapMaterial->SetVectorParameterValue(FName("UnwrapLocation"), UnwrapLocation);
	UnwrapMaterial->SetVectorParameterValue(FName("ObjectPosition"), ObjectLocation);
	UnwrapMaterial->SetVectorParameterValue(FName("ObjectBounds"), BoxExtent);
}

void UCanvasComponent::DrawPoint(const FVector& WorldLocation, float Value, UStaticMeshComponent* MeshComponent)
{
	const FBoxSphereBounds MyBounds = MeshComponent->Bounds;
	// 获取物体包围盒的大小
	FVector BoxExtent = MyBounds.BoxExtent*2;

	//存的时候存相对坐标总行了吧
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

	UTextureRenderTarget2D* LocalRenderTarget = RTDisplayIR;  // 把成员变量拷贝到局部变量
	TMap<FVector, float> LocalDrawnPoints = DrawnPoints;

	FIntPoint Size = FIntPoint(LocalRenderTarget->SizeX, LocalRenderTarget->SizeY);
	FTextureRenderTargetResource* RenderTargetResource = LocalRenderTarget->GameThread_GetRenderTargetResource();

	TWeakObjectPtr<UCanvasComponent> WeakThis(this);
	TWeakObjectPtr<UStaticMeshComponent> WeakMeshComponent(MeshComponent); // 确保 MeshComponent 传进来
	
	ENQUEUE_RENDER_COMMAND(RunInterpolationCS)(
	[this,WeakThis, WeakMeshComponent,Size,RenderTargetResource, LocalDrawnPoints, BoxExtent](FRHICommandListImmediate& RHICmdList)
	{

		// 1. 获取输入纹理 SRV	
		FTexture2DRHIRef RenderTargetRHI = RenderTargetResource->GetRenderTargetTexture();
		FShaderResourceViewRHIRef InputTextureSRV = RHICreateShaderResourceView(RenderTargetRHI, 0);

if (!RenderTargetRHI.IsValid())
{
	UE_LOG(LogTemp, Error, TEXT("RenderTargetRHI is invalid!"));
}

		// 2. 上传关键点数据
		FRWBufferStructured GPUKeyBuffer;
		TArray<FVector4f> KeyPointArray;
		for (const auto& Pair : LocalDrawnPoints)
		{
			KeyPointArray.Add(FVector4f(Pair.Key.X, Pair.Key.Y, Pair.Key.Z, Pair.Value));
		}

		UE_LOG(LogTemp, Log, TEXT("Uploading %d keypoints:"), LocalDrawnPoints.Num());

		for (const auto& Pair : LocalDrawnPoints)
		{
			const FVector& Pos = Pair.Key;
			float Val = Pair.Value;
			UE_LOG(LogTemp, Log, TEXT("  KeyPoint: (%f, %f, %f), Value: %f"), Pos.X, Pos.Y, Pos.Z, Val);

			KeyPointArray.Add(FVector4f(Pos.X, Pos.Y, Pos.Z, Val));
		}
		
		if (KeyPointArray.Num() > 0)
		{
			GPUKeyBuffer.Initialize(TEXT("KeyBuffer"), sizeof(FVector4f), KeyPointArray.Num(), BUF_ShaderResource | BUF_Static);
			void* BufferData = RHILockBuffer(GPUKeyBuffer.Buffer, 0, sizeof(FVector4f) * KeyPointArray.Num(), RLM_WriteOnly);
			FMemory::Memcpy(BufferData, KeyPointArray.GetData(), sizeof(FVector4f) * KeyPointArray.Num());
			RHIUnlockBuffer(GPUKeyBuffer.Buffer);
		}
		
		// 3. 创建输出纹理 UAV
		// 尺寸同输入纹理
		EPixelFormat PixelFormat = PF_R8G8B8A8; // 或者 PF_R32G32B32A32_FLOAT，视需求而定PF_R32_FLOAT
		FRHIResourceCreateInfo CreateInfo(TEXT("OutputTexture"));
		// 创建描述结构体
		FRHITextureCreateDesc Desc = FRHITextureCreateDesc::Create2D(TEXT("MyRWTexture"))
			.SetExtent(Size.X, Size.Y)
			.SetFormat(PixelFormat)
			.SetFlags(TexCreate_ShaderResource | TexCreate_UAV | TexCreate_RenderTargetable)
			.SetNumMips(1);
		FTexture2DRHIRef OutputTexture = RHICreateTexture(Desc)->GetTexture2D();
		FUnorderedAccessViewRHIRef OutputTextureUAV = RHICmdList.CreateUnorderedAccessView(OutputTexture);
		// 可选）创建 SRV（只读访问，适用于 Shader 中 Texture2D<> 输入）
		FShaderResourceViewRHIRef OutputTextureSRV = RHICreateShaderResourceView(OutputTexture, 0);
		
		// 4. 调用 Dispatch
		FAPCSParameters ShaderParameters;
		ShaderParameters.NumKeyPoints = LocalDrawnPoints.Num();
		// 由于 GPU Shader 一般使用 float 类型，Shader 参数结构体中也应使用 FVector3f
		ShaderParameters.BoxExtent = FVector3f(BoxExtent);
		ShaderParameters.KeyPositions = GPUKeyBuffer;
		ShaderParameters.OutputTexture = OutputTextureUAV;
		ShaderParameters.InputTexture = InputTextureSRV;
		ShaderParameters.TextureWidth = Size.X;
		ShaderParameters.TextureHeight = Size.Y;

		FAPCSManager::Dispatch(RHICmdList, ShaderParameters);
		
		// 读取 OutputTexture 的数据并传回 CPU
	   FTextureUtils::ReadTextureToCPU(OutputTexture, [WeakThis, WeakMeshComponent](TArray<FColor>& PixelData, int32 Width, int32 Height)
	   {
		   // 在 GameThread 中执行
		   AsyncTask(ENamedThreads::GameThread, [WeakThis, WeakMeshComponent, PixelData, Width, Height]()
		   {
			   if (!WeakThis.IsValid() || !WeakMeshComponent.IsValid()) return;

			   // 创建 Texture2D
			   UTexture2D* Texture = FTextureUtils::CreateTexture2DFromRaw(PixelData, Width, Height);

			   // 应用到材质
			   WeakThis->ApplyTextureToMaterial(WeakMeshComponent.Get(), Texture);
		   	   FString FilePath = FPaths::ProjectContentDir() + TEXT("Textures/OutputTexture.png");
		   	   FTextureUtils::SaveTextureToDisk(Texture,FilePath);
		   });
	   });
		// 如何把生成的纹理应用回模型
		//5. 可选：导出纹理
		// FString FilePath = FPaths::ProjectContentDir() + TEXT("Textures/OutputTexture.png");
		// SaveTextureToDisk(OutputTexture, FilePath);
	});
	}




        	





// 	ENQUEUE_RENDER_COMMAND(UploadKeyPoints)(
// [KeyPointArray, &GPUKeyBuffer](FRHICommandListImmediate& RHICmdList)
// {
// void* BufferData = RHICmdList.LockBuffer(
// 	GPUKeyBuffer.Buffer,
// 	0,
// 	sizeof(FVector4f) * KeyPointArray.Num(),
// 	RLM_WriteOnly
// );
//
// FMemory::Memcpy(BufferData, KeyPointArray.GetData(), sizeof(FVector4f) * KeyPointArray.Num());
//
// RHICmdList.UnlockBuffer(GPUKeyBuffer.Buffer);
// });
	//3.显示结果
	//CopyRenderTargetRHI(RenderTarget,RTDisplayIR );
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


void UCanvasComponent::UploadKeyPointsToGPU(const TArray<FVector4f>& KeyPoints)
{
	
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

   //      	// 创建计算着色器的参数
			// FAPCSParameters ShaderParameters;
			// ShaderParameters.VertexPositions = PositionBufferSRV;
			// ShaderParameters.VertexUVs = UVBufferSRV;
			// ShaderParameters.OutputTexture = OutputTextureUAV;
			// ShaderParameters.VertexCount = VertexPositionsCopy->Num();
			// ShaderParameters.MinBound = FVector3f(0.0f, 0.0f, 0.0f);  // 这里使用适当的包围盒值
			// ShaderParameters.MaxBound = FVector3f(1.0f, 1.0f, 1.0f);  // 这里使用适当的包围盒值
   //      	
   //      	// 使用获取的包围盒的最小值和最大值
   //      	// 手动计算包围盒的 Min 和 Max
			// FVector3f MinBound = FVector3f(MeshBounds.Origin - MeshBounds.BoxExtent);
			// FVector3f MaxBound = FVector3f(MeshBounds.Origin + MeshBounds.BoxExtent);
	  //
			// ShaderParameters.MinBound = MinBound;
			// ShaderParameters.MaxBound = MaxBound;
   //      	
			// ShaderParameters.TextureWidth = Width;
			// ShaderParameters.TextureHeight = Height;
   //      	
			// FAPCSManager::Dispatch(RHICmdList, ShaderParameters);
	  //
   //      	FString FilePath = FPaths::ProjectContentDir() + TEXT("Textures/OutputTexture.png");
	  //
   //      	SaveTextureToDisk(OutputTexture, FilePath);
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

// 将生成的纹理结果换给模型
void UCanvasComponent::ApplyTextureToMaterial(UStaticMeshComponent* MeshComponent, UTexture2D* GeneratedTexture)
{
	if (!MeshComponent || !IRMaterial || !GeneratedTexture) return;

	// 设置材质参数
	IRMaterial->SetTextureParameterValue(TEXT("TextureParam"), GeneratedTexture);

	// 应用到所有材质槽
	int32 SlotCount = MeshComponent->GetNumMaterials();
	for (int32 i = 0; i < SlotCount; ++i)
	{
		MeshComponent->SetMaterial(i, IRMaterial);
	}
}

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
				// 确保 SaveTextureToDisk 在游戏线程中执行 所以能在渲染线程内调用
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

void UCanvasComponent::CopyRenderTargetRHI(UTextureRenderTarget2D* SourceRT, UTextureRenderTarget2D* DestRT)
{
	ENQUEUE_RENDER_COMMAND(CopyRTToRT)(
		[SourceRT, DestRT](FRHICommandListImmediate& RHICmdList)
		{
			if (!SourceRT || !DestRT) return;

			FTextureRenderTargetResource* SrcResource = SourceRT->GameThread_GetRenderTargetResource();
			FTextureRenderTargetResource* DstResource = DestRT->GameThread_GetRenderTargetResource();

			if (!SrcResource || !DstResource) return;

			FRHITexture2D* SrcTexture = SrcResource->GetRenderTargetTexture();
			FRHITexture2D* DstTexture = DstResource->GetRenderTargetTexture();

			if (!SrcTexture || !DstTexture) return;

			FRHICopyTextureInfo CopyInfo;
			CopyInfo.Size = SrcTexture->GetSizeXYZ();
			RHICmdList.CopyTexture(SrcTexture, DstTexture, CopyInfo);
		}
	);
}

