// Operation Greyfield - Phase 2 (Mass Entity backbone). Combat for entities that may have NO
// actor at all - the far/background tier that Mass's representation LOD keeps as data-only.
// Combat-capable "hero" tier units (near camera/selected) still fight via the existing
// Actor+GAS path (see AGreyfieldUnit); this processor is what lets everyone else still fight
// without needing a full actor, ability system component, etc.
//
// Targeting uses a spatial hash grid (Engine's own THierarchicalHashGrid2D, same tool Mass's
// own avoidance/navigation systems use internally) instead of an O(n^2) per-unit scan over
// every other unit - the "spatial hash grid" piece of the Phase 2 architecture. Scan is
// throttled (see ScanInterval) rather than run every frame - the "simulation LOD" piece.
// See Build Log, Phase 2, for how this fits the rest of the lightweight-at-scale design.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityQuery.h"
#include "GreyfieldMassCombatProcessor.generated.h"

UCLASS()
class PROJECTGREYFIELD_API UGreyfieldMassCombatProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UGreyfieldMassCombatProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;

	// Combat targeting re-scans on this cadence rather than every frame - irrelevant at small
	// scale, necessary at 2000+. Damage-over-time (TimeSinceLastAttack vs AttackInterval) still
	// accumulates every scan, so attack rate isn't distorted by the throttle.
	static constexpr float ScanInterval = 0.25f;
	float TimeSinceLastScan = 0.f;
};
