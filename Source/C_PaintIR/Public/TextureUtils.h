#pragma once

class C_PAINTIR_API FTextureUtils
{
public:
	// 生成纹理的函数（逻辑待补充）
	static UTexture2D* GenerateTexture();

	// 保存纹理到磁盘
	static bool SaveTextureToDisk(UTexture2D* Texture, const FString& FilePath);
};
