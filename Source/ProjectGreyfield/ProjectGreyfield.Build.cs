// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectGreyfield : ModuleRules
{
	public ProjectGreyfield(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "GameplayTags", "AIModule", "NavigationSystem", "GameplayAbilities", "GameplayTasks", "UMG", "Slate", "SlateCore",
			// Phase 2: Mass Entity backbone for at-scale unit simulation (2000+ target).
			"MassEntity", "MassCore", "MassCommon", "MassSpawner", "MassSimulation", "MassMovement", "MassRepresentation", "MassLOD", "MassActors",
			"MassNavigation", "MassNavMeshNavigation" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
