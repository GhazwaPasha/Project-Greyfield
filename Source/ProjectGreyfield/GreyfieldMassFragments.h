// Operation Greyfield - Phase 2 (Mass Entity backbone). Plain-data fragments carried by every
// Mass-simulated unit. Kept separate from the traits/processors that use them, matching the
// engine's own MassMovementFragments.h / MassNavigationFragments.h convention.
//
// Design note (2026-08-24, target raised to 2000+ units): these fragments deliberately hold only
// what's needed for an entity that may have NO actor at all (far/culled units still need to know
// their team, health, and squad role). Combat-capable "hero" tier units still get a full
// AGreyfieldUnit actor (GAS/ASC) via Mass's representation system - this file's job is to keep
// the far/background tier alive and simulate-able without one.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Mass/EntityHandle.h" // FMassEntityHandle - moved here from MassEntity in UE 5.8, lives in the MassCore module
#include "GreyfieldUnit.h" // EGreyfieldTeam
#include "GreyfieldMassFragments.generated.h"

// Per-entity mutable combat/identity state. Every Mass-simulated Greyfield unit carries exactly one.
USTRUCT()
struct FGreyfieldMassAgentFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	EGreyfieldTeam Team = EGreyfieldTeam::Player;

	UPROPERTY()
	float CurrentHealth = 100.f;

	// Seconds remaining before this entity may attack again. Mirrors AGreyfieldUnit's
	// TimeSinceLastAttack/AttackInterval cooldown pattern so the eventual Mass-side combat
	// processor behaves identically to the existing Actor-side combat loop.
	UPROPERTY()
	float TimeSinceLastAttack = 0.f;

	// Entity currently being attack-moved toward / auto-engaged. Invalid handle = no target.
	UPROPERTY()
	FMassEntityHandle AttackTargetEntity;
};

// Per-unit-type constant stats, shared across every entity spawned from the same
// UGreyfieldMassAgentTrait config (mirrors FMassMovementParameters' const-shared-fragment pattern
// rather than duplicating these floats per entity instance).
USTRUCT()
struct FGreyfieldMassAgentParams : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Greyfield|Combat")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, Category = "Greyfield|Combat")
	float AttackDamage = 10.f;

	UPROPERTY(EditAnywhere, Category = "Greyfield|Combat")
	float AttackRange = 400.f;

	UPROPERTY(EditAnywhere, Category = "Greyfield|Combat")
	float AttackInterval = 1.5f;

	// Fog of war - matches AGreyfieldUnit::VisionRadius so Mass-simulated and Actor-simulated
	// units read the same to the existing vision/fog system.
	UPROPERTY(EditAnywhere, Category = "Greyfield|Vision")
	float VisionRadius = 1200.f;
};

// Squad membership. A "squad" is the unit selected together in the one move/attack-move order -
// only the leader entity ever issues a real NavMesh path query; followers just chase the leader's
// resolved position offset by their formation slot. This is the single biggest lever for hitting
// 2000+ units without a pathfinding-cost cliff (see Build Log, Phase 2).
USTRUCT()
struct FGreyfieldSquadFragment : public FMassFragment
{
	GENERATED_BODY()

	// Identifies which squad this entity belongs to. 0 = unassigned / solo (moves via NavMesh
	// like a leader of a squad-of-one).
	UPROPERTY()
	int32 SquadId = 0;

	// True for exactly one entity per SquadId - the only one with a NavMesh Navigation trait
	// and therefore the only one that ever queries the NavMesh.
	UPROPERTY()
	bool bIsSquadLeader = true;

	// Offset from the leader's location this entity should trail at, in the leader's facing
	// space (X = forward, Y = right). Assigned once when the squad is formed.
	UPROPERTY()
	FVector2D FormationOffset = FVector2D::ZeroVector;

	// Cached leader handle so the formation processor doesn't need a lookup table rebuilt
	// every tick. Invalid/self for leaders.
	UPROPERTY()
	FMassEntityHandle LeaderEntity;
};
