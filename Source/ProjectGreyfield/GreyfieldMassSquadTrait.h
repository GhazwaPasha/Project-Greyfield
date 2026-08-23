// Operation Greyfield - Phase 2 (Mass Entity backbone). Marks an entity as squad-capable.
// Deliberately its own trait, separate from UGreyfieldMassAgentTrait, since squad formation
// is a movement concept, not a combat one (a future non-combat unit could use it too).

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "GreyfieldMassSquadTrait.generated.h"

UCLASS(meta = (DisplayName = "Greyfield Squad Member"))
class PROJECTGREYFIELD_API UGreyfieldMassSquadTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
