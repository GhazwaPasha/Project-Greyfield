// Operation Greyfield - Phase 2 (Mass Entity backbone).

#include "GreyfieldMassCombatProcessor.h"
#include "GreyfieldMassFragments.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "HierarchicalHashGrid2D.h"

namespace
{
	// 2 levels, 4x ratio, 500cm finest cell - mirrors the engine's own
	// FNavigationObstacleHashGrid2D (MassNavigationSubsystem.h) sizing, which is tuned for
	// exactly this kind of per-unit gameplay query at RTS-battlefield scale.
	using FGreyfieldCombatHashGrid = THierarchicalHashGrid2D<2, 4, int32>;

	struct FCombatCandidate
	{
		FMassEntityHandle Handle;
		FVector Position = FVector::ZeroVector;
		EGreyfieldTeam Team = EGreyfieldTeam::Player;
	};
}

UGreyfieldMassCombatProcessor::UGreyfieldMassCombatProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::AllNetModes);
	ProcessingPhase = EMassProcessingPhase::PostPhysics;
}

void UGreyfieldMassCombatProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FGreyfieldMassAgentFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FGreyfieldMassAgentParams>();
	EntityQuery.RegisterWithProcessor(*this);
}

void UGreyfieldMassCombatProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Throttled re-scan rather than every frame - see header comment / Build Log Phase 2.
	TimeSinceLastScan += Context.GetDeltaTimeSeconds();
	if (TimeSinceLastScan < ScanInterval)
	{
		return;
	}
	const float ElapsedSinceLastScan = TimeSinceLastScan;
	TimeSinceLastScan = 0.f;

	// Pass 1: collect every living combat entity's position/team, and drop each into the
	// *opposing* team's spatial hash grid keyed by its index into that flat array - so a query
	// from one side only ever touches candidates it could actually attack.
	TArray<FCombatCandidate> Candidates;
	FGreyfieldCombatHashGrid PlayerGrid(500.f);
	FGreyfieldCombatHashGrid EnemyGrid(500.f);

	EntityQuery.ForEachEntityChunk(Context, [&Candidates, &PlayerGrid, &EnemyGrid](FMassExecutionContext& ChunkContext)
	{
		const TArrayView<FGreyfieldMassAgentFragment> Agents = ChunkContext.GetMutableFragmentView<FGreyfieldMassAgentFragment>();
		const TConstArrayView<FTransformFragment> Transforms = ChunkContext.GetFragmentView<FTransformFragment>();

		for (int32 Index = 0; Index < ChunkContext.GetNumEntities(); ++Index)
		{
			if (Agents[Index].CurrentHealth <= 0.f)
			{
				continue; // Dead, awaiting deferred destruction below.
			}

			FCombatCandidate Candidate;
			Candidate.Handle = ChunkContext.GetEntity(Index);
			Candidate.Position = Transforms[Index].GetTransform().GetLocation();
			Candidate.Team = Agents[Index].Team;

			const int32 CandidateIndex = Candidates.Add(Candidate);
			const FBox PointBounds(Candidate.Position, Candidate.Position);

			if (Candidate.Team == EGreyfieldTeam::Player)
			{
				PlayerGrid.Add(CandidateIndex, PointBounds);
			}
			else
			{
				EnemyGrid.Add(CandidateIndex, PointBounds);
			}
		}
	});

	// Pass 2: each entity queries the opposing grid for the nearest candidate in range and
	// fires on cooldown. Damage applied directly to the health fragment, mirroring the
	// existing Actor-side GAS combat's own simplification (see Phase 4's known gaps) rather
	// than introducing a second, inconsistent damage model.
	EntityQuery.ForEachEntityChunk(Context, [&EntityManager, &Candidates, &PlayerGrid, &EnemyGrid, ElapsedSinceLastScan](FMassExecutionContext& ChunkContext)
	{
		const TArrayView<FGreyfieldMassAgentFragment> Agents = ChunkContext.GetMutableFragmentView<FGreyfieldMassAgentFragment>();
		const TConstArrayView<FTransformFragment> Transforms = ChunkContext.GetFragmentView<FTransformFragment>();
		const FGreyfieldMassAgentParams& Params = ChunkContext.GetConstSharedFragment<FGreyfieldMassAgentParams>();

		for (int32 Index = 0; Index < ChunkContext.GetNumEntities(); ++Index)
		{
			FGreyfieldMassAgentFragment& Agent = Agents[Index];
			if (Agent.CurrentHealth <= 0.f)
			{
				continue;
			}

			Agent.TimeSinceLastAttack += ElapsedSinceLastScan;

			const FVector Position = Transforms[Index].GetTransform().GetLocation();
			const FGreyfieldCombatHashGrid& OpposingGrid = (Agent.Team == EGreyfieldTeam::Player) ? EnemyGrid : PlayerGrid;

			const FBox QueryBounds(Position - FVector(Params.AttackRange), Position + FVector(Params.AttackRange));
			TArray<int32> NearbyIndices;
			OpposingGrid.Query(QueryBounds, NearbyIndices);

			int32 BestCandidateIndex = INDEX_NONE;
			float BestDistSq = FMath::Square(Params.AttackRange);
			for (const int32 CandidateIndex : NearbyIndices)
			{
				const FCombatCandidate& Candidate = Candidates[CandidateIndex];
				const float DistSq = FVector::DistSquared(Position, Candidate.Position);
				if (DistSq <= BestDistSq)
				{
					BestDistSq = DistSq;
					BestCandidateIndex = CandidateIndex;
				}
			}

			if (BestCandidateIndex == INDEX_NONE || Agent.TimeSinceLastAttack < Params.AttackInterval)
			{
				continue; // Nothing in range, or still on cooldown.
			}

			Agent.TimeSinceLastAttack = 0.f;
			const FMassEntityHandle TargetHandle = Candidates[BestCandidateIndex].Handle;
			if (FGreyfieldMassAgentFragment* TargetAgent = EntityManager.GetFragmentDataPtr<FGreyfieldMassAgentFragment>(TargetHandle))
			{
				TargetAgent->CurrentHealth -= Params.AttackDamage;
			}
		}
	});

	// Pass 3: destroy anything that died this scan. Deferred - archetypes can't be mutated
	// (which destroying an entity does) while a query is actively iterating them.
	EntityQuery.ForEachEntityChunk(Context, [&Context](FMassExecutionContext& ChunkContext)
	{
		const TConstArrayView<FGreyfieldMassAgentFragment> Agents = ChunkContext.GetFragmentView<FGreyfieldMassAgentFragment>();
		for (int32 Index = 0; Index < ChunkContext.GetNumEntities(); ++Index)
		{
			if (Agents[Index].CurrentHealth <= 0.f)
			{
				Context.Defer().DestroyEntity(ChunkContext.GetEntity(Index));
			}
		}
	});
}
