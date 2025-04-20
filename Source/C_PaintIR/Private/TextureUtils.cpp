#include "TextureUtils.h"
#include "ImageUtils.h"
#include "Engine/Texture2D.h"

UTexture2D* FTextureUtils::GenerateTexture()
{
	// 定义纹理的尺寸
	int32 Width = 256;
	int32 Height = 256;

	// 创建临时纹理
	UTexture2D* NewTexture = UTexture2D::CreateTransient(Width, Height);

	if (NewTexture)
	{
		// 配置纹理
		NewTexture->MipGenSettings = TMGS_NoMipmaps;
		NewTexture->CompressionSettings = TC_VectorDisplacementmap;
		NewTexture->SRGB = false;

		// 锁定纹理数据进行写入
		FTexture2DMipMap& Mip = NewTexture->GetPlatformData()->Mips[0];
		void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);

		// 获取像素数据指针
		FColor* FormattedImageData = static_cast<FColor*>(TextureData);

		// 填充纹理数据为纯白色
		for (int32 y = 0; y < Height; ++y)
		{
			for (int32 x = 0; x < Width; ++x)
			{
				FormattedImageData[x + y * Width] = FColor::Red;
			}
		}

		// 解锁纹理数据
		Mip.BulkData.Unlock();

		// 更新纹理资源
		NewTexture->UpdateResource();
	}

	return NewTexture;
}

// eg "/Game/Textures/Test.png"
bool FTextureUtils::SaveTextureToDisk(UTexture2D* Texture, const FString& FilePath)
{
	if (!Texture)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid texture."));
		return false;
	}

	// 获取纹理的宽度和高度
	int32 Width = Texture->GetSizeX();
	int32 Height = Texture->GetSizeY();

	// 创建一个数组来存储像素数据
	TArray<FColor> Bitmap;
	Bitmap.SetNumUninitialized(Width * Height);

	// 获取纹理的像素数据
	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	void* TextureData = Mip.BulkData.Lock(LOCK_READ_ONLY);
	FColor* FormattedImageData = static_cast<FColor*>(TextureData);

	// 将像素数据复制到 Bitmap 数组
	FMemory::Memcpy(Bitmap.GetData(), FormattedImageData, Bitmap.Num() * sizeof(FColor));

	// 解锁纹理数据
	Mip.BulkData.Unlock();

	// 将 Bitmap 数组保存为 PNG 文件
	TArray<uint8> PNGData;
	FImageUtils::CompressImageArray(Width, Height, Bitmap, PNGData);
	return FFileHelper::SaveArrayToFile(PNGData, *FilePath);
}

void FTextureUtils::SaveTextureToDisk(FTexture2DRHIRef Texture, const FString& FilePath)
{
	ReadTextureToCPU(Texture, [FilePath](TArray<FColor>& Data, int32 Width, int32 Height)
	{
		SaveImageToDisk(Data, Width, Height, FilePath);
	});
}

// ReadTextureToCPU() 是异步的，需要将输出纹理的转换回调设计为异步链式逻辑
void FTextureUtils::ConvertRHIToTexture2D(FTexture2DRHIRef TextureRHI, TFunction<void(UTexture2D*)> OnTextureReady)
{
	int32 Width = TextureRHI->GetSizeX();
	int32 Height = TextureRHI->GetSizeY();

	TSharedRef<TArray<FColor>> OutColorData = MakeShared<TArray<FColor>>();
	OutColorData->SetNumUninitialized(Width * Height);

	ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
		[TextureRHI, OutColorData, Width, Height, OnTextureReady](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.ReadSurfaceData(TextureRHI, FIntRect(0, 0, Width, Height), *OutColorData, FReadSurfaceDataFlags());

			AsyncTask(ENamedThreads::GameThread, [OutColorData, Width, Height, OnTextureReady]()
			{
				UTexture2D* Texture2D = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
				Texture2D->MipGenSettings = TMGS_NoMipmaps;
				Texture2D->SRGB = false;

				void* TextureMemory = Texture2D->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
				FMemory::Memcpy(TextureMemory, OutColorData->GetData(), Width * Height * sizeof(FColor));
				Texture2D->GetPlatformData()->Mips[0].BulkData.Unlock();

				Texture2D->UpdateResource();

				OnTextureReady(Texture2D);
			});
		}
	);
}

//ReadTexture → PixelData → CreateTexture2D
void FTextureUtils::ReadTextureToCPU(FTexture2DRHIRef Texture, TFunction<void(TArray<FColor>&, int32 Width, int32 Height)> OnReadComplete)
{
	int32 Width = Texture->GetSizeX();
	int32 Height = Texture->GetSizeY();
	TSharedRef<TArray<FColor>> OutImageData = MakeShared<TArray<FColor>>();
	OutImageData->SetNumUninitialized(Width * Height);

	ENQUEUE_RENDER_COMMAND(ReadTextureData)(
		[Texture, OutImageData, Width, Height, OnReadComplete](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.ReadSurfaceData(Texture, FIntRect(0, 0, Width, Height), *OutImageData, FReadSurfaceDataFlags());

			// 回到游戏线程
			AsyncTask(ENamedThreads::GameThread, [OutImageData, Width, Height, OnReadComplete]()
			{
				OnReadComplete(*OutImageData, Width, Height);
			});
		}
	);
}

// 工具：从 RawData 创建 Texture2D（已提供）
UTexture2D* FTextureUtils::CreateTexture2DFromRaw(const TArray<FColor>& ImageData, int32 Width, int32 Height)
{
	UTexture2D* Texture2D = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	Texture2D->MipGenSettings = TMGS_NoMipmaps;
	Texture2D->SRGB = false;

	// 填充数据
	void* TextureData = Texture2D->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, ImageData.GetData(), Width * Height * sizeof(FColor));
	Texture2D->GetPlatformData()->Mips[0].BulkData.Unlock();
	Texture2D->UpdateResource();

	return Texture2D;
}

void FTextureUtils::SaveImageToDisk(const TArray<FColor>& ImageData, int32 Width, int32 Height, const FString& FilePath)
{
	TArray<uint8> CompressedData;
	FImageUtils::CompressImageArray(Width, Height, ImageData, CompressedData);

	if (FFileHelper::SaveArrayToFile(CompressedData, *FilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("Texture saved to %s"), *FilePath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save texture to %s"), *FilePath);
	}
}

