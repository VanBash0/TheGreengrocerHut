using UnrealBuildTool;

public class TheGreengrocerHutEditor : ModuleRules
{
	public TheGreengrocerHutEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"TheGreengrocerHut"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd",
			"PropertyEditor",
			"SlateCore",
			"Slate",
            "InputCore"
        });
	}
}