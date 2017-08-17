// Copyright (c) Henry Cooney 2017 

using UnrealBuildTool;
using System.Collections.Generic;

public class IKDemoEditorTarget : TargetRules
{
	public IKDemoEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange( new string[] { "IKDemo" } );
	}
}
