// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class UnrealPlusEditorTarget : TargetRules
{
	public UnrealPlusEditorTarget(TargetInfo Target) : base(Target)
	{
        bDisableDebugInfo = true;
		Type = TargetType.Editor;
		ExtraModuleNames.Add("UnrealPlus");
	}
}
