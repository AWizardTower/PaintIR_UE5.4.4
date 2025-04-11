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