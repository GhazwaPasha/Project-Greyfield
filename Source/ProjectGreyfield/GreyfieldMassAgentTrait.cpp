// Operation Greyfield - Phase 2 (Mass Entity backbone).

#include "GreyfieldMassAgentTrait.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityUtils.h"
#include "MassCommonFragments.h"
#include "Engine/World.h"

void UGreyfieldMassAgentTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	// Per-entity mutable state, seeded from this trait's constant params so every entity spawned
	// from this config starts at full health on its configured team.
	FGreyfieldMassAgentFragment& AgentFragment = BuildContext.AddFragment_GetRef<FGreyfieldMassAgentFragment>();
	AgentFragment.Team = DefaultTeam;
	AgentFragment.CurrentHealth = AgentParams.MaxHealth;

	// Constant per-unit-type stats, shared across every entity using this exact config rather
	// than duplicated per instance.
	const FConstSharedStruct ParamsFragment = EntityManager.GetOrCreateConstSharedFragment(AgentParams);
	BuildContext.AddConstSharedFragment(ParamsFragment);
}
