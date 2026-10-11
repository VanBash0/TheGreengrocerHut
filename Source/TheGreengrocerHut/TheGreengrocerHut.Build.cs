// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class TheGreengrocerHut : ModuleRules
{
	public TheGreengrocerHut(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // DlgDialogue.h is included from public headers, so dependents (e.g. the Editor module) need the path too
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "..", "..", "Plugins", "DlgSystem", "Source", "DlgSystem"));

        PublicDependencyModuleNames.AddRange(new string[]
		{ "Core", "CoreUObject", "Engine", "InputCore", "StructUtils",
			"SubstanceCore", "DeveloperSettings", "UMG", "GameplayTags",
            "Paper2D", "DlgSystem"});

		PrivateDependencyModuleNames.AddRange(new string[] { "AssetRegistry" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}