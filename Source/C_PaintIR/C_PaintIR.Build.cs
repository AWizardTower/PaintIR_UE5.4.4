// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class C_PaintIR : ModuleRules
{
	public C_PaintIR(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "RenderCore", "RHI", "ProceduralMeshComponent", "ImageWrapper","UMG"});

		PrivateDependencyModuleNames.AddRange(new string[] { "CustomShadersDeclarations" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
