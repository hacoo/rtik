// Copyright (c) Henry Cooney 2017

using UnrealBuildTool;

public class IKDemo : ModuleRules
{
	public IKDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "rtik" });

		PrivateDependencyModuleNames.AddRange(new string[] { "rtik" });

        // DynamicallyLoadedModuleNames.AddRange(new string[] { "rtik" });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
