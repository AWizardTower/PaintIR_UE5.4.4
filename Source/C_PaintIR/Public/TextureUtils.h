#pragma once

class C_PAINTIR_API FTextureUtils
{
public:
	// 生成纹理的函数（逻辑待补充）
	static UTexture2D* GenerateTexture();

	static UTexture2D* CreateTexture2DFromRaw(const TArray<FColor>& ImageData, int32 Width, int32 Height);
	// 保存纹理到磁盘
	static bool SaveTextureToDisk(UTexture2D* Texture, const FString& FilePath);

	static void SaveShaderOutputTexture(FTexture2DRHIRef OutputTexture);

	static void ReadTextureToCPU(FTexture2DRHIRef Texture, TFunction<void(TArray<FColor>&, int32 Width, int32 Height)> OnReadComplete);

	static void SaveImageToDisk(const TArray<FColor>& ImageData, int32 Width, int32 Height, const FString& FilePath);

	static void SaveTextureToDisk(FTexture2DRHIRef Texture, const FString& FilePath);

	static void ConvertRHIToTexture2D(FTexture2DRHIRef TextureRHI, TFunction<void(UTexture2D*)> OnTextureReady);
};
