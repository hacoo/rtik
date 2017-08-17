// Copyright (c) Henry Cooney 2017

using UnrealBuildTool;
using System.Collections.Generic;

public class IKDemoTarget : TargetRules
{
	public IKDemoTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.AddRange( new string[] { "IKDemo" } );
	}
}