// Operation Greyfield - Phase 2 (Mass Entity backbone). The squad-level pathfinding piece:
// only a squad's leader entity carries a NavMesh Navigation trait and ever queries the NavMesh.
// This processor keeps every follower's move target trailing the leader's actual resolved
// position (offset by formation slot) instead of each follower pathing independently - the
// single biggest lever for reaching 2000+ units without a pathfinding-cost cliff.
// See Build Log, Phase 2, for the full lightweight-at-scale architecture this is one piece of.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityQuery.h"
#include "GreyfieldMassSquadFormationProcessor.generated.h"

UCLASS()
class PROJECTGREYFIELD_API UGreyfieldSquadFormationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UGreyfieldSquadFormationProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;

	// Proof-of-movement logging (Phase 2 PIE verification, 2026-08-29) - logs one leader's and one
	// of its followers' resolved world positions every tick for the first LogTickBudget ticks after
	// a squad first appears, then stops (cheap dev-visibility, not a permanent per-frame cost).
	// Every-tick rather than throttled-by-seconds deliberately: headless verification runs via
	// -ExecutePythonScript get auto-killed (QUIT_EDITOR) well under 1 real second after PIE starts
	// (documented, unexplained engine behavior - see PROJECT_STATUS.md), so a handful of dense
	// early samples is the only way to actually see a position trend in that window.
	int32 LogTicksRemaining = 15;
};
