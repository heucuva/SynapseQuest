// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SynapseQuest : ModuleRules
{
	public SynapseQuest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"Synapse",
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"SynapseQuest",
			"SynapseQuest/Dialogue",
			"SynapseQuest/Dialogue/UI",
			"SynapseQuest/Variant_Horror",
			"SynapseQuest/Variant_Horror/UI",
			"SynapseQuest/Variant_Shooter",
			"SynapseQuest/Variant_Shooter/AI",
			"SynapseQuest/Variant_Shooter/UI",
			"SynapseQuest/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] { "SynapseEditor" });
        }
    }
}
