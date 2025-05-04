// Fill out your copyright notice in the Description page of Project Settings.


#include "CanvasComponent.h"

#include "CanvasSaveManager.h"
#include "GlobalSettingsManager.h"
#include "ImageUtils.h"
#include "MyGameInstance.h"
#include "PointSaveGame.h"
#include "Engine/Texture2D.h"
#include "TextureUtils.h"
#include "Engine/Texture.h"
#include "CustomShadersDeclarations/Private/APCSDeclaration.h"

#include "RHICommandList.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetRenderingLibrary.h"

// Sets default values for this component's properties
UCanvasComponent::UCanvasComponent()
{
	// 使用 GlobalSettingsManager 获取设置
	if (GlobalSettingsManager)
	{
		ETextureSize CurrentSize = GlobalSettingsManager->GetTextureSize();
		FValueRange Range = GlobalSettingsManager->GetTextureSizeRange();
        
		// 根据获取的设置进行相关处理
	}
	// 创建子组件并附加
	KeyPointVisualizer = CreateDefaultSubobject<UPointVisualizerComponent>(TEXT("KeyPointVisualizer"));
	KeyPointVisualizer->SetupAttachment(this);
	
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
		
		FVector Location(0.0f, 0.0f, 2.0f);
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

	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> RenderTargetObj(TEXT("TextureRenderTarget2D'/Game/Dilation/RT_Dilated.RT_Dilated'")); // 路径注意要写对
	if (RenderTargetObj.Succeeded())
	{
		DefaultRenderTarget = RenderTargetObj.Object;
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
		BackgroundPlane->SetRelativeLocation(FVector(0.0f, 0.0f, 1.0f));
		BackgroundPlane->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

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
	KeyPointVisualizer->OwningCanvas = this;

	// 从 GameInstance 获取全局设置管理器
	GlobalSettingsManager = Cast<UMyGameInstance>(GetWorld()->GetGameInstance())->GetGlobalSettingsManager();
	if (GlobalSettingsManager)
	{
		GlobalSettingsManager->OnTexureSizeChanged.AddDynamic(this, &UCanvasComponent::OnGlobalSettingsChanged);
	}
}

void UCanvasComponent::OnGlobalSettingsChanged()
{
	UE_LOG(LogTemp, Warning, TEXT("Global settings changed! Refreshing RenderTarget..."));
	ApplyTextureSize();
}

// Called every frame
void UCanvasComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCanvasComponent::InitializeForMesh()
{
	int32 TextureSize = GlobalSettingsManager->GetTextureSizeValue();
	if (!MeshComponent) return;
	UE_LOG(LogTemp, Log, TEXT("初始化了组件"));
	RenderTarget = NewObject<UTextureRenderTarget2D>(this);
	RenderTarget->InitAutoFormat(TextureSize, TextureSize);
	RenderTarget->ClearColor = FLinearColor::Black;
	RenderTarget->UpdateResourceImmediate();
	
	SceneCapture->TextureTarget = RenderTarget;
	SceneCapture->OrthoWidth = TextureSize; 

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
	UnwrapMaterial->SetScalarParameterValue(FName("UnwrapScale"), TextureSize);
	UnwrapMaterial->SetVectorParameterValue(FName("UnwrapLocation"), UnwrapLocation);
	UnwrapMaterial->SetVectorParameterValue(FName("ObjectPosition"), ObjectLocation);
	UnwrapMaterial->SetVectorParameterValue(FName("ObjectBounds"), BoxExtent);
	
	UStaticMesh* PlaneMesh = BackgroundPlane->GetStaticMesh();
	if (PlaneMesh)
	{
		FBoxSphereBounds MeshBounds = PlaneMesh->GetBounds();
		FVector PlaneMeshSize = MeshBounds.BoxExtent * 2.0f; // 得到原始尺寸（长宽高）
		BackgroundPlane->SetRelativeScale3D(FVector(TextureSize/PlaneMeshSize.X, TextureSize/PlaneMeshSize.Y, 1.0f)); // 适当放大
		// PlaneMeshSize.X 就是平面长度
		// PlaneMeshSize.Y 就是平面宽度
	}

	if (RenderTarget)
	{
		// 确保不显示脏数据
		UKismetRenderingLibrary::ClearRenderTarget2D(this, RenderTarget, FLinearColor::Black);
	}

	// CaptureWithUnwrapAndRestore();
	//如果不再每次展开了，就不能更新纹理了，显然对于每一个画布都需要单独的rendertarget实例了
}

void UCanvasComponent::ApplyTextureSize()
{
	int32 TextureSize = GlobalSettingsManager->GetTextureSizeValue();
	if (!RenderTarget) return;
    
	// 更新RenderTarget尺寸
	RenderTarget->InitAutoFormat(TextureSize, TextureSize);
    
	// 更新SceneCapture正交相机宽度
	if (SceneCapture)
	{
		SceneCapture->OrthoWidth = TextureSize;
	}
    
	// 更新Unwrap材质参数
	if (UnwrapMaterial)
	{
		// FVector UnwrapLocation = GetComponentLocation() + FVector(0.0f, 0.0f, 2.0f);
		// FVector ObjectLocation = MeshComponent->GetComponentLocation();
		// FVector BoxExtent = MeshComponent->Bounds.BoxExtent * 2;

		UnwrapMaterial->SetScalarParameterValue(FName("UnwrapScale"), TextureSize);
		// UnwrapMaterial->SetVectorParameterValue(FName("UnwrapLocation"), UnwrapLocation);
		// UnwrapMaterial->SetVectorParameterValue(FName("ObjectPosition"), ObjectLocation);
		// UnwrapMaterial->SetVectorParameterValue(FName("ObjectBounds"), BoxExtent);
	}

	// 更新背景Plane缩放
	if (BackgroundPlane)
	{
		UStaticMesh* PlaneMesh = BackgroundPlane->GetStaticMesh();
		if (PlaneMesh)
		{
			FBoxSphereBounds MeshBounds = PlaneMesh->GetBounds();
			FVector PlaneMeshSize = MeshBounds.BoxExtent * 2.0f;
			if (!PlaneMeshSize.IsNearlyZero())
			{
				BackgroundPlane->SetRelativeScale3D(
					FVector(TextureSize / PlaneMeshSize.X, TextureSize / PlaneMeshSize.Y, 1.0f)
				);
			}
		}
	}
    
	// 清空一下渲染目标，防止脏数据
	UKismetRenderingLibrary::ClearRenderTarget2D(this, RenderTarget, FLinearColor::Black);
}


void UCanvasComponent::DrawPoint(const FVector& WorldLocation, float Value)
{
	const FBoxSphereBounds MyBounds = MeshComponent->Bounds;
	FVector BoxExtent = MyBounds.BoxExtent * 2;
	//FVector LocalLocation = GetComponentTransform().InverseTransformPosition(WorldLocation);
	FVector LocalPointLocation = GetComponentTransform().InverseTransformPosition(WorldLocation);
	FVector LocalBoxCenter = GetComponentTransform().InverseTransformPosition(MyBounds.Origin);
	FVector LocalLocation = (LocalPointLocation - LocalBoxCenter);

	UE_LOG(LogTemp, Log, TEXT("Clicked at local position: %s, and current value is :%f"), *LocalLocation.ToString(), Value);

	DrawnPoints.Add(LocalLocation, Value);

	// 可视化关键点
	if (KeyPointVisualizer)
	{
		KeyPointVisualizer->AddKeyPoint(LocalPointLocation, Value);
	}

	// 然后直接刷新纹理
	GenerateTextureFromDrawnPoints();
}

void UCanvasComponent::GenerateTextureFromDrawnPoints()
{
	if (DrawnPoints.Num() == 0)
	{
		return;
	}

    if (!MeshComponent) return;
    UStaticMesh* StaticMesh = MeshComponent->GetStaticMesh();
    if (!StaticMesh) return;

    // 1. 展开UV
    //ApplyUnwrapMaterial();

    // 2. 捕获场景
    //SceneCapture->CaptureScene();
	
    // 3. 开始生成纹理
    UTextureRenderTarget2D* LocalRenderTarget = DefaultRenderTarget;
    TMap<FVector, float> LocalDrawnPoints = DrawnPoints;
    FVector BoxExtent = MeshComponent->Bounds.BoxExtent * 2;

    FIntPoint Size(LocalRenderTarget->SizeX, LocalRenderTarget->SizeY);
	
	// 打印生成纹理的大小
	if (GeneratedIRTexture)
	{
		UE_LOG(LogTemp, Log, TEXT("Generated Texture Size: Width = %d, Height = %d"), LocalRenderTarget->SizeX, LocalRenderTarget->SizeY);
	}
	
    FTextureRenderTargetResource* RenderTargetResource = LocalRenderTarget->GameThread_GetRenderTargetResource();

    TWeakObjectPtr<UCanvasComponent> WeakThis(this);
    TWeakObjectPtr<UStaticMeshComponent> WeakMeshComponent(MeshComponent);

    ENQUEUE_RENDER_COMMAND(RunInterpolationCS)(
        [this, WeakThis, WeakMeshComponent, Size, RenderTargetResource, LocalDrawnPoints, BoxExtent](FRHICommandListImmediate& RHICmdList)
        {
            // 1. 获取输入纹理 SRV
            FTexture2DRHIRef RenderTargetRHI = RenderTargetResource->GetRenderTargetTexture();
            if (!RenderTargetRHI.IsValid())
            {
                UE_LOG(LogTemp, Error, TEXT("RenderTargetRHI is invalid!"));
                return;
            }
            FShaderResourceViewRHIRef InputTextureSRV = RHICreateShaderResourceView(RenderTargetRHI, 0);

            // 2. 上传关键点数据
            FRWBufferStructured GPUKeyBuffer;
            TArray<FVector4f> KeyPointArray;
            for (const auto& Pair : LocalDrawnPoints)
            {
                KeyPointArray.Add(FVector4f(Pair.Key.X, Pair.Key.Y, Pair.Key.Z, Pair.Value));
            }
            if (KeyPointArray.Num() > 0)
            {
                GPUKeyBuffer.Initialize(TEXT("KeyBuffer"), sizeof(FVector4f), KeyPointArray.Num(), BUF_ShaderResource | BUF_Static);
                void* BufferData = RHILockBuffer(GPUKeyBuffer.Buffer, 0, sizeof(FVector4f) * KeyPointArray.Num(), RLM_WriteOnly);
                FMemory::Memcpy(BufferData, KeyPointArray.GetData(), sizeof(FVector4f) * KeyPointArray.Num());
                RHIUnlockBuffer(GPUKeyBuffer.Buffer);
            }

            // 3. 创建输出纹理 UAV
            EPixelFormat PixelFormat = PF_R8G8B8A8;
            FRHITextureCreateDesc Desc = FRHITextureCreateDesc::Create2D(TEXT("MyRWTexture"))
                .SetExtent(Size.X, Size.Y)
                .SetFormat(PixelFormat)
                .SetFlags(TexCreate_ShaderResource | TexCreate_UAV | TexCreate_RenderTargetable)
                .SetNumMips(1);

            FTexture2DRHIRef OutputTexture = RHICreateTexture(Desc)->GetTexture2D();
            FUnorderedAccessViewRHIRef OutputTextureUAV = RHICmdList.CreateUnorderedAccessView(OutputTexture);

            // 4. 调用 Dispatch
            FAPCSParameters ShaderParameters;
            ShaderParameters.NumKeyPoints = LocalDrawnPoints.Num();
            ShaderParameters.BoxExtent = FVector3f(BoxExtent);
            ShaderParameters.KeyPositions = GPUKeyBuffer;
            ShaderParameters.OutputTexture = OutputTextureUAV;
            ShaderParameters.InputTexture = InputTextureSRV;
            ShaderParameters.TextureWidth = Size.X;
            ShaderParameters.TextureHeight = Size.Y;

            FAPCSManager::Dispatch(RHICmdList, ShaderParameters);

            // 5. 读取并回到CPU，生成Texture2D
            FTextureUtils::ReadTextureToCPU(OutputTexture, [this,WeakThis, WeakMeshComponent](TArray<FColor>& PixelData, int32 Width, int32 Height)
            {
                AsyncTask(ENamedThreads::GameThread, [this,WeakThis, WeakMeshComponent, PixelData, Width, Height]()
                {
                    if (!WeakThis.IsValid() || !WeakMeshComponent.IsValid()) return;

                    GeneratedIRTexture = FTextureUtils::CreateTexture2DFromRaw(PixelData, Width, Height);
                    WeakThis->ApplyTextureToMaterial(GeneratedIRTexture);
                	
                });
            });
        }
    );
}

// void UCanvasComponent::DrawPoint(const FVector& WorldLocation, float Value, UStaticMeshComponent* MeshComponent)
// {
// 	const FBoxSphereBounds MyBounds = MeshComponent->Bounds;
// 	// 获取物体包围盒的大小
// 	FVector BoxExtent = MyBounds.BoxExtent*2;
//
// 	//存的时候存相对坐标总行了吧
// 	// 将世界坐标转换为组件的局部坐标
// 	FVector LocalLocation = GetComponentTransform().InverseTransformPosition(WorldLocation);
// 	UE_LOG(LogTemp, Log, TEXT("Clicked at local position: %s, and current value is :%f"), *LocalLocation.ToString(), Value);
//
// 	DrawnPoints.Add(LocalLocation, Value);
//
// 	// 可视化关键点
// 	if (KeyPointVisualizer)
// 	{
// 		KeyPointVisualizer->AddKeyPoint(LocalLocation, Value);
// 	}
//
// 	UStaticMesh* StaticMesh = MeshComponent->GetStaticMesh();
// 	if (!StaticMesh) return;
//
// 	//CaptureWithUnwrapAndRestore();
//
// 	//1. 展UV
// 	ApplyUnwrapMaterial(MeshComponent);
//
// 	//2. 捕获
// 	SceneCapture->CaptureScene();
//
// 	UTextureRenderTarget2D* LocalRenderTarget = RTDisplayIR;  // 把成员变量拷贝到局部变量
// 	TMap<FVector, float> LocalDrawnPoints = DrawnPoints;
//
// 	FIntPoint Size = FIntPoint(LocalRenderTarget->SizeX, LocalRenderTarget->SizeY);
// 	FTextureRenderTargetResource* RenderTargetResource = LocalRenderTarget->GameThread_GetRenderTargetResource();
//
// 	TWeakObjectPtr<UCanvasComponent> WeakThis(this);
// 	TWeakObjectPtr<UStaticMeshComponent> WeakMeshComponent(MeshComponent); // 确保 MeshComponent 传进来
// 	
// 	ENQUEUE_RENDER_COMMAND(RunInterpolationCS)(
// 	[this,WeakThis, WeakMeshComponent,Size,RenderTargetResource, LocalDrawnPoints, BoxExtent](FRHICommandListImmediate& RHICmdList)
// 	{
//
// 		// 1. 获取输入纹理 SRV	
// 		FTexture2DRHIRef RenderTargetRHI = RenderTargetResource->GetRenderTargetTexture();
// 		FShaderResourceViewRHIRef InputTextureSRV = RHICreateShaderResourceView(RenderTargetRHI, 0);
//
// if (!RenderTargetRHI.IsValid())
// {
// 	UE_LOG(LogTemp, Error, TEXT("RenderTargetRHI is invalid!"));
// }
//
// 		// 2. 上传关键点数据
// 		FRWBufferStructured GPUKeyBuffer;
// 		TArray<FVector4f> KeyPointArray;
// 		for (const auto& Pair : LocalDrawnPoints)
// 		{
// 			KeyPointArray.Add(FVector4f(Pair.Key.X, Pair.Key.Y, Pair.Key.Z, Pair.Value));
// 		}
//
// 		UE_LOG(LogTemp, Log, TEXT("Uploading %d keypoints:"), LocalDrawnPoints.Num());
//
// 		for (const auto& Pair : LocalDrawnPoints)
// 		{
// 			const FVector& Pos = Pair.Key;
// 			float Val = Pair.Value;
// 			UE_LOG(LogTemp, Log, TEXT("  KeyPoint: (%f, %f, %f), Value: %f"), Pos.X, Pos.Y, Pos.Z, Val);
//
// 			KeyPointArray.Add(FVector4f(Pos.X, Pos.Y, Pos.Z, Val));
// 		}
// 		
// 		if (KeyPointArray.Num() > 0)
// 		{
// 			GPUKeyBuffer.Initialize(TEXT("KeyBuffer"), sizeof(FVector4f), KeyPointArray.Num(), BUF_ShaderResource | BUF_Static);
// 			void* BufferData = RHILockBuffer(GPUKeyBuffer.Buffer, 0, sizeof(FVector4f) * KeyPointArray.Num(), RLM_WriteOnly);
// 			FMemory::Memcpy(BufferData, KeyPointArray.GetData(), sizeof(FVector4f) * KeyPointArray.Num());
// 			RHIUnlockBuffer(GPUKeyBuffer.Buffer);
// 		}
// 		
// 		// 3. 创建输出纹理 UAV
// 		// 尺寸同输入纹理
// 		EPixelFormat PixelFormat = PF_R8G8B8A8; // 或者 PF_R32G32B32A32_FLOAT，视需求而定PF_R32_FLOAT
// 		FRHIResourceCreateInfo CreateInfo(TEXT("OutputTexture"));
// 		// 创建描述结构体
// 		FRHITextureCreateDesc Desc = FRHITextureCreateDesc::Create2D(TEXT("MyRWTexture"))
// 			.SetExtent(Size.X, Size.Y)
// 			.SetFormat(PixelFormat)
// 			.SetFlags(TexCreate_ShaderResource | TexCreate_UAV | TexCreate_RenderTargetable)
// 			.SetNumMips(1);
// 		FTexture2DRHIRef OutputTexture = RHICreateTexture(Desc)->GetTexture2D();
// 		FUnorderedAccessViewRHIRef OutputTextureUAV = RHICmdList.CreateUnorderedAccessView(OutputTexture);
// 		// 可选）创建 SRV（只读访问，适用于 Shader 中 Texture2D<> 输入）
// 		FShaderResourceViewRHIRef OutputTextureSRV = RHICreateShaderResourceView(OutputTexture, 0);
// 		
// 		// 4. 调用 Dispatch
// 		FAPCSParameters ShaderParameters;
// 		ShaderParameters.NumKeyPoints = LocalDrawnPoints.Num();
// 		// 由于 GPU Shader 一般使用 float 类型，Shader 参数结构体中也应使用 FVector3f
// 		ShaderParameters.BoxExtent = FVector3f(BoxExtent);
// 		ShaderParameters.KeyPositions = GPUKeyBuffer;
// 		ShaderParameters.OutputTexture = OutputTextureUAV;
// 		ShaderParameters.InputTexture = InputTextureSRV;
// 		ShaderParameters.TextureWidth = Size.X;
// 		ShaderParameters.TextureHeight = Size.Y;
//
// 		FAPCSManager::Dispatch(RHICmdList, ShaderParameters);
// 		
// 		// 读取 OutputTexture 的数据并传回 CPU
// 	   FTextureUtils::ReadTextureToCPU(OutputTexture, [WeakThis, WeakMeshComponent](TArray<FColor>& PixelData, int32 Width, int32 Height)
// 	   {
// 		   // 在 GameThread 中执行
// 		   AsyncTask(ENamedThreads::GameThread, [WeakThis, WeakMeshComponent, PixelData, Width, Height]()
// 		   {
// 			   if (!WeakThis.IsValid() || !WeakMeshComponent.IsValid()) return;
//
// 			   // 创建 Texture2D
// 			   UTexture2D* Texture = FTextureUtils::CreateTexture2DFromRaw(PixelData, Width, Height);
//
// 			   // 应用到材质
// 			   WeakThis->ApplyTextureToMaterial(WeakMeshComponent.Get(), Texture);
// 		   	   //FString FilePath = FPaths::ProjectContentDir() + TEXT("Textures/OutputTexture.png");
// 		   	   //FTextureUtils::SaveTextureToDisk(Texture,FilePath);
// 		   });
// 	   });
//
// 		//把生成结果存到类变量中即可
// 		// 如何把生成的纹理应用回模型
// 		//5. 可选：导出纹理
// 		// FString FilePath = FPaths::ProjectContentDir() + TEXT("Textures/OutputTexture.png");
// 		// SaveTextureToDisk(OutputTexture, FilePath);
// 	});
// }

void UCanvasComponent::ModifyPointValue(const FVector& WorldLocation, float NewValue)
{
	if (DrawnPoints.Contains(WorldLocation))
	{
		DrawnPoints[WorldLocation] = NewValue;
		//UE_LOG(LogTemp, Log, TEXT("找到了这个节点"));
	}

	GenerateTextureFromDrawnPoints();
}

void UCanvasComponent::RemovePoint(const FVector& WorldLocation)
{
	if (DrawnPoints.Contains(WorldLocation-GetComponentTransform().InverseTransformPosition(MeshComponent->Bounds.Origin)))
	{
		DrawnPoints.Remove(WorldLocation-GetComponentTransform().InverseTransformPosition(MeshComponent->Bounds.Origin));
		UE_LOG(LogTemp, Log, TEXT("删除了这个节点"));
	}
	
	GenerateTextureFromDrawnPoints();
}
// 	const float Tolerance = 1e-2f;
//
// 	for (auto& Elem : DrawnPoints)
// 	{
// 		if (FVector::DistSquared(Elem.Key, WorldLocation) < Tolerance * Tolerance)
// 		{
// 			Elem.Value = NewValue;
// 			UE_LOG(LogTemp, Log, TEXT("Key point updated at %s to value %.2f"), *WorldLocation.ToString(), NewValue);
// 			return;
// 		}
// 	}
//
// 	UE_LOG(LogTemp, Warning, TEXT("ModifyKeyPointValue failed: point not found at %s"), *WorldLocation.ToString());


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

TArray<UMaterialInterface*> UCanvasComponent::ApplyUnwrapMaterial()
{
	// 保存原始材质
	TArray<UMaterialInterface*> OriginalMaterialsArray;
	
	if (MeshComponent && UnwrapMaterial)
	{
		int32 MaterialCount = MeshComponent->GetNumMaterials();
		for (int32 Index = 0; Index < MaterialCount; ++Index)
		{
			OriginalMaterialsArray.Add(MeshComponent->GetMaterial(Index));
		}
		
		for (int32 Index = 0; Index < MaterialCount; ++Index)
		{
			MeshComponent->SetMaterial(Index, UnwrapMaterial);
		}
		
	}

	return OriginalMaterialsArray;
}

void UCanvasComponent::CaptureWithUnwrapAndRestore()
{
	// 获取父 Actor
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	// 获取 StaticMeshComponent
	//UStaticMeshComponent* MeshComponent = OwnerActor->FindComponentByClass<UStaticMeshComponent>();
	//if (!MeshComponent) return;
	
	TArray<UMaterialInterface*> OriginalMaterialsArray = ApplyUnwrapMaterial();

	// 调用捕捉（会在 GPU 异步进行）
	SceneCapture->CaptureScene();

	// 延迟一帧再 CaptureScene
	// FTimerHandle DummyHandle;
	// GetWorld()->GetTimerManager().SetTimer(DummyHandle, [this]()
	// {
	// 	SceneCapture->CaptureScene();
	// 	// 此时 RenderTarget 绑定的 UI Image 会在下帧更新，画面正确
	// }, 0.01f, false);



	// 延迟一帧再恢复原材质，确保 GPU 有时间完成渲染
	// FTimerHandle TimerHandle;
	// GetWorld()->GetTimerManager().SetTimer(TimerHandle, [MeshComponent, OriginalMaterialsArray]()
	// {
	// 	// 恢复原始材质
	// 	for (int32 Index = 0; Index < OriginalMaterialsArray.Num(); ++Index)
	// 	{
	// 		MeshComponent->SetMaterial(Index, OriginalMaterialsArray[Index]);
	// 	}
	// }, 0.05f, false); // 0.05 秒通常等于一帧
}

// 将生成的纹理结果换给模型
void UCanvasComponent::ApplyTextureToMaterial(UTexture2D* GeneratedTexture)
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

void UCanvasComponent::SetMeshMaterial()
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

void UCanvasComponent::LoadFromKeyPointData(const FKeyPointData& Data)
{
	// 1. 覆盖已有数据
	DrawnPoints = Data.Points;

	UE_LOG(LogTemp, Warning, TEXT("加载到 CanvasComponent 的关键点数量: %d"), DrawnPoints.Num());

	for (const TPair<FVector, float>& Pair : DrawnPoints)
	{
		UE_LOG(LogTemp, Warning, TEXT("关键点位置: %s，值: %.2f"), *Pair.Key.ToString(), Pair.Value);
	}

	//2. 刷新可视化组件（如果你有可视化）
	if (KeyPointVisualizer)
	{
		KeyPointVisualizer->UpdateAllVisualizers(DrawnPoints);
	}

	//刷新
	GenerateTextureFromDrawnPoints();
}

bool UCanvasComponent::ExportTextureToDisk(const FString& FilePath)
{
	if (!GeneratedIRTexture)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExportTextureToDisk: GeneratedIRTexture is null"));
		return false;
	}

	bool bSaved = FTextureUtils::SaveTextureToDisk(GeneratedIRTexture, FilePath);
	if (!bSaved)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExportTextureToDisk: Failed to save texture to %s"), *FilePath);
	}
	return bSaved;
}