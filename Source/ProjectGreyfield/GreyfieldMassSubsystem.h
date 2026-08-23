// Operation Greyfield - Phase 2 (Mass Entity backbone). Gameplay-facing entry point for
// ordering Mass-simulated squads around - the equivalent of AGreyfieldUnit::MoveToLocation, but
// for entities that may not have an actor at all. Wiring the *existing* selection/order UI to
// call this for Mass-driven units is the next integration step, tracked as a known gap - this
// subsystem is the landing pad for that, not the UI change itself.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "Mass/EntityHandle.h" // FMassEntityHandle - moved here from MassEntity in UE 5.8, lives in the MassCore module
#include "GreyfieldMassSubsystem.generated.h"

UCLASS()
class PROJECTGREYFIELD_API UGreyfieldMassSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Designates SquadEntities[0] as the squad leader and arranges the rest into a simple grid
	// formation around it. Call once when a squad is first grouped (e.g. on selection), not on
	// every move order.
	void FormSquad(const TArray<FMassEntityHandle>& SquadEntities);

	// Issues a move order to an already-formed squad: only the leader gets a real NavMesh path
	// request (via CreateNewAction), followers pick it up next tick through
	// UGreyfieldSquadFormationProcessor.
	void IssueSquadMoveOrder(const TArray<FMassEntityHandle>& SquadEntities, const FVector& Destination);

	// DEV/TEST ONLY (Phase 2 verification, 2026-08-24) - not part of the real game. Spawns Count
	// entities from Config directly via UMassSpawnerSubsystem (bypassing the AMassSpawner actor
	// entirely - no level placement needed), spreads them in a small grid around SpawnOrigin,
	// forms them into one squad, and issues a move order to Destination. Proves
	// FormSquad/IssueSquadMoveOrder/UGreyfieldSquadFormationProcessor actually work in PIE without
	// needing the real player-order UI wired up yet. Logs entity count and chosen leader so
	// results are checkable via the output log.
	UFUNCTION(BlueprintCallable, Category = "Greyfield|Mass|Test")
	void SpawnAndTestSquad(class UMassEntityConfigAsset* Config, int32 Count, const FVector& SpawnOrigin, const FVector& Destination);
};
