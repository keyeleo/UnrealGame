// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Iot : ModuleRules
{
	public Iot(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine", "InputCore", "Http",
            "PakFile",
            "APEX",
            "PhysX",
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "Slate", "SlateCore", "UMG",
            "Http",
            "slua_unreal", "slua_profile",
            "MediaAssets"
        });

        PrivateIncludePathModuleNames.AddRange(new string[] {
            "slua_unreal"
        });

        PublicIncludePathModuleNames.AddRange(new string[] {
            "slua_unreal", "slua_profile"
        });
	}
}
