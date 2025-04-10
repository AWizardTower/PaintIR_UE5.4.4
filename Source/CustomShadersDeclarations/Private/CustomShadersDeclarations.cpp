#include "CustomShadersDeclarations.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "GlobalShader.h"

#define LOCTEXT_NAMESPACE "FCustomShadersDeclarationsModule"

void FCustomShadersDeclarationsModule::StartupModule()
{
    // Maps virtual shader source directory to actual shaders directory on disk.
	// 要在文件管理器中添加文件夹，不要在ide中添加！
	FString ShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders/Private"));
	AddShaderSourceDirectoryMapping("/CustomShaders", ShaderDirectory);
}

void FCustomShadersDeclarationsModule::ShutdownModule()
{
    
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FCustomShadersDeclarationsModule, CustomShadersDeclarations)