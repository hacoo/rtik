// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class ue4ikEditor : ModuleRules
{
	public ue4ikEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;


        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" , "UnrealEd" });

        PrivateDependencyModuleNames.AddRange(new string[] { "EditorStyle", "AnimGraph", "BlueprintGraph", "PropertyEditor", "Slate", "SlateCore" });

        PublicIncludePaths.AddRange(
            new string[] {
                "ue4ikEditor/Public"
            });

        PrivateIncludePaths.AddRange(
            new string[] {
                "ue4ikEditor/Private"
            });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
