// Operation Greyfield - Phase 2 (Mass Entity backbone).

#include "GreyfieldMassSquadTrait.h"
#include "GreyfieldMassFragments.h"
#include "MassEntityTemplateRegistry.h"

void UGreyfieldMassSquadTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FGreyfieldSquadFragment>();
}
