// Copyright (c) Henry Cooney 2017

using UnrealBuildTool;

public class rtikEditor : ModuleRules
{
	public rtikEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        // PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivatePCHHeaderFile = "rtikEditor.h";

        PublicDependencyModuleNames.AddRange(new string[] { "rtik", "Core", "CoreUObject", "Engine", "InputCore" , "UnrealEd" });

        PrivateDependencyModuleNames.AddRange(new string[] { "EditorStyle", "AnimGraph", "BlueprintGraph", "PropertyEditor", "Slate", "SlateCore" });

        // PublicIncludePaths.AddRange(new string[] { "rtikEditor/Public", "rtikEditor/Public/GraphNodes" });

        // PrivateIncludePaths.AddRange(new string[] { "rtikEditor/Private", "rtikEditor/Private/GraphNodes" });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
