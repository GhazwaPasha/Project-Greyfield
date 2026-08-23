// Operation Greyfield - Phase 2 (Mass Entity backbone). Adds Greyfield's combat/team/vision
// data to a Mass entity config. Composed alongside Epic's stock Movement/Steering/Avoidance/
// NavMesh-Navigation traits in the unit's UMassEntityConfigAsset - this trait only adds what's
// actually Greyfield-specific.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "GreyfieldMassFragments.h"
#include "GreyfieldMassAgentTrait.generated.h"

UCLASS(meta = (DisplayName = "Greyfield Agent"))
class PROJECTGREYFIELD_API UGreyfieldMassAgentTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	// Starting team for entities spawned from this config. A spawner can still override this
	// per-instance later (e.g. AI-owned spawners overriding to Enemy) - this is just the default.
	UPROPERTY(EditAnywhere, Category = "Greyfield|Combat")
	EGreyfieldTeam DefaultTeam = EGreyfieldTeam::Player;

	UPROPERTY(EditAnywhere, Category = "Greyfield|Combat")
	FGreyfieldMassAgentParams AgentParams;

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
