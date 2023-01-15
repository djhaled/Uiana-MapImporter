// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UianaCPP : ModuleRules
{
	public UianaCPP(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
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
				"Core",
				"UnrealPSKPSA"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Settings",
				"Slate",
				"RenderCore",
				"EditorStyle",
				"SlateCore",
				"Json",
				"JsonUtilities",
				"EditorScriptingUtilities",
				"AssetTools",
				"LevelEditor",
				"MaterialEditor"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		BuildVersion Version;
		if (BuildVersion.TryRead(BuildVersion.GetDefaultFileName(), out Version))
		{
			if (Version.MajorVersion == 5)
			{
				// do ue 5 stuff
				PrivateDependencyModuleNames.Add("EditorFramework");
			}
			else
			{
				PrivateDependencyModuleNames.AddRange(
					new string[]
					{
						"EditorStyle",
						"EditorSubsystem"
					});
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
