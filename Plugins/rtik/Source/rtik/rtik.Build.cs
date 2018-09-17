// Copyright (c) Henry Cooney 2017

using UnrealBuildTool;

public class rtik : ModuleRules
{
    public rtik(ReadOnlyTargetRules Target) : base(Target)
    {     
        // PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AnimGraph", "BlueprintGraph", "AnimGraphRuntime", "AnimationCore" });

        PrivateDependencyModuleNames.AddRange(new string[] {  });

        // PublicIncludePaths.AddRange(new string[] { "rtik/Public", "rtik/Public/IK", "rtik/Public/Utility" });

        // PrivateIncludePaths.AddRange(new string[] { "rtik/Private", "rtik/Private/IK", "rtik/Private/Utility"});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
