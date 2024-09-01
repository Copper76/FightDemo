// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FightDemo : ModuleRules
{
	public FightDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
            new string[] {
                "FightDemo/Public",
                "FightDemo/Misc"
            }
        );

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
