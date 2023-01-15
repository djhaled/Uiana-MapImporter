// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UnrealPSKPSA : ModuleRules
{
	public UnrealPSKPSA(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"ProceduralMeshComponent",
				"UnrealEd",
				"Projects",
				"MeshDescription",
				"RawMesh",
				"RenderCore",
				"SlateCore",
				"Slate",
				"MeshBuilder",
				"MeshUtilitiesCommon", 
				"EditorScriptingUtilities",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		BuildVersion Version;
		if (BuildVersion.TryRead(BuildVersion.GetDefaultFileName(), out Version))
		{
			if (Version.MajorVersion == 5)
			{
				// do ue 5 stuff
				
			}
			else
			{
				PrivateDependencyModuleNames.Add("GeometricObjects");
				// do ue 4 stuff
			}
		}
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
