using UnrealBuildTool;
using System.IO;

public class SocketPreviewPlugin : ModuleRules
{
	public SocketPreviewPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd",
			"Slate",
			"SlateCore",
			"EditorSubsystem",
			"LevelEditor",
			"AssetRegistry",
			"ContentBrowser",
			"ToolMenus",
			"SkeletalMeshEditor",
			"SkeletonEditor",
			"Persona",
			"InputCore",
			"RooftopGarden"
		});

	}
}