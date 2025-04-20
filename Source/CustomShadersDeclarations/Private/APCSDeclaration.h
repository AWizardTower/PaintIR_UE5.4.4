#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

struct FAPCSParameters
{
	// 纹理渲染目标，用于存储计算结果
	UTextureRenderTarget2D* RenderTarget;

	// 获取渲染目标的大小
	FIntPoint GetRenderTargetSize() const
	{
		return CachedRenderTargetSize;
	}

	// 顶点数量
	uint32 NumKeyPoints;

	// 包围盒最小值与最大值
	// FVector3f MinBound;
	// FVector3f MaxBound;
	FVector3f BoxExtent;

	// 纹理的宽度和高度 不在着色器里使用，仅在调度时用
	uint32 TextureWidth;
	uint32 TextureHeight;

	// 顶点位置和 UV 缓冲区
	// FShaderResourceViewRHIRef VertexPositions;
	// FShaderResourceViewRHIRef VertexUVs;
	FRWBufferStructured KeyPositions;
	
	FShaderResourceViewRHIRef InputTexture;
	
	// 输出纹理的 UAV（Unordered Access View）
	FUnorderedAccessViewRHIRef OutputTexture;

	// 时间戳
	uint32 TimeStamp;

	// 构造函数
	FAPCSParameters()
		: RenderTarget(nullptr),
		  NumKeyPoints(0),
		  //MinBound(FVector3f::ZeroVector),
		  //MaxBound(FVector3f::ZeroVector),
		  TextureWidth(0),
		  TextureHeight(0),
	      BoxExtent(FVector3f::ZeroVector),
		  TimeStamp(0)
	{}

	FAPCSParameters(UTextureRenderTarget2D* IORenderTarget)
		: RenderTarget(IORenderTarget),
	      NumKeyPoints(0),
		//   MinBound(FVector3f::ZeroVector),
		//   MaxBound(FVector3f::ZeroVector),
		  TextureWidth(0),
		  TextureHeight(0),
	      BoxExtent(FVector3f::ZeroVector),
		  TimeStamp(0)
	{
		// 获取渲染目标的大小
		CachedRenderTargetSize = RenderTarget ? FIntPoint(RenderTarget->SizeX, RenderTarget->SizeY) : FIntPoint::ZeroValue;
	}

private:
	// 缓存的渲染目标大小
	FIntPoint CachedRenderTargetSize;
};

class CUSTOMSHADERSDECLARATIONS_API FAPCSManager
{
	public:
	static void Dispatch(
		FRHICommandListImmediate& RHICmdList,
		const FAPCSParameters& Parameters,
		uint32 ThreadGroupSizeX = 8,
		uint32 ThreadGroupSizeY = 8
	);
};
