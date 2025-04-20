#include "APCSDeclaration.h"

#include "GlobalShader.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "Modules/ModuleManager.h"

class FAPCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FAPCS);
	SHADER_USE_PARAMETER_STRUCT(FAPCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(uint32, NumKeyPoints)

		//SHADER_PARAMETER(FVector3f, MinBound)
		//SHADER_PARAMETER(FVector3f, MaxBound)
		SHADER_PARAMETER(FVector3f, BoxExtent)

		SHADER_PARAMETER_SRV(Texture2D<FVector4f>, InputTexture)
		//SHADER_PARAMETER(uint32, TextureWidth)
		//SHADER_PARAMETER(uint32, TextureHeight)

		SHADER_PARAMETER_SRV(StructuredBuffer<FVector4f>, KeyPositions)
		//SHADER_PARAMETER_SRV(StructuredBuffer<FVector3f>, VertexPositions)
		//SHADER_PARAMETER_SRV(StructuredBuffer<FVector2f>, VertexUVs)

		SHADER_PARAMETER_UAV(RWTexture2D<FVector4f>, OutputTexture)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};

IMPLEMENT_GLOBAL_SHADER(FAPCS, "/CustomShaders/AttributeInterpolationCS.usf", "MainCS", SF_Compute);

void FAPCSManager::Dispatch(
		FRHICommandListImmediate& RHICmdList,
		const FAPCSParameters& Parameters,
		uint32 ThreadGroupSizeX,
		uint32 ThreadGroupSizeY
	)
{
	TShaderMapRef<FAPCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	// 创建参数结构体，并将参数传递给着色器
	FAPCS::FParameters ShaderParams;
	ShaderParams.NumKeyPoints = Parameters.NumKeyPoints;
	// ShaderParams.MinBound = Parameters.MinBound;
	// ShaderParams.MaxBound = Parameters.MaxBound;
	// ShaderParams.TextureWidth = Parameters.TextureWidth;
	// ShaderParams.TextureHeight = Parameters.TextureHeight;
	// ShaderParams.VertexPositions = Parameters.VertexPositions;
	// ShaderParams.VertexUVs = Parameters.VertexUVs;
	ShaderParams.BoxExtent = Parameters.BoxExtent;
	ShaderParams.KeyPositions = Parameters.KeyPositions.SRV;
	ShaderParams.InputTexture = Parameters.InputTexture;
	ShaderParams.OutputTexture = Parameters.OutputTexture;


	// 设置着色器参数
	SetShaderParameters(RHICmdList, ComputeShader, ComputeShader.GetComputeShader(), ShaderParams);
	
	// 正确计算 Dispatch 尺寸
	FIntVector DispatchSize(
		FMath::DivideAndRoundUp(Parameters.TextureWidth, ThreadGroupSizeX),
		FMath::DivideAndRoundUp(Parameters.TextureHeight, ThreadGroupSizeY),
		1
	);

	FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, ShaderParams, DispatchSize);

	// 清除 UAV（Unordered Access View）资源
	UnsetShaderUAVs(RHICmdList, ComputeShader, ComputeShader.GetComputeShader());
}