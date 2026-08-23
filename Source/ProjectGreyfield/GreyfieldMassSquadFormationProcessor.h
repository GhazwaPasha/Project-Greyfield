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
};
