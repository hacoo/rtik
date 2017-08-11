// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class rtikTarget : TargetRules
{
	public rtikTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.AddRange( new string[] { "rtik" } );

        if (UEBuildConfiguration.bBuildEditor)
        {
            ExtraModuleNames.Add("rtikEditor");
        }
	}
}
