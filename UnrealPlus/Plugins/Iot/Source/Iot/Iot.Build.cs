// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Iot : ModuleRules
{
	public Iot(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Http", "PakFile" });

        PrivateDependencyModuleNames.AddRange(new string[] { "slua_unreal", "slua_profile", "Slate", "SlateCore", "UMG", "Http", "MediaAssets" });

        PrivateIncludePathModuleNames.AddRange(new string[] { "slua_unreal" });

        PublicIncludePathModuleNames.AddRange(new string[] { "slua_unreal", "slua_profile" });
	}
}
