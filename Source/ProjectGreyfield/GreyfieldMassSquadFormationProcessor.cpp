// Operation Greyfield - Phase 2 (Mass Entity backbone).

#include "GreyfieldMassSquadFormationProcessor.h"
#include "GreyfieldMassFragments.h"
#include "MassCommonFragments.h"
#include "MassNavigationFragments.h"
#include "MassExecutionContext.h"

UGreyfieldSquadFormationProcessor::UGreyfieldSquadFormationProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::AllNetModes);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void UGreyfieldSquadFormationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FGreyfieldSquadFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UGreyfieldSquadFormationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Pass 1: record every squad leader's current resolved position/facing. Leaders are the only
	// entities that ever queried the NavMesh this tick (via their NavMesh Navigation trait) -
	// followers just chase this, never issuing a NavMesh query of their own.
	struct FLeaderInfo
	{
		FVector Position = FVector::ZeroVector;
		FQuat FacingQuat = FQuat::Identity;
		float DesiredSpeed = 0.f;
	};
	TMap<FMassEntityHandle, FLeaderInfo> LeaderInfoByHandle;

	EntityQuery.ForEachEntityChunk(Context, [&LeaderInfoByHandle](FMassExecutionContext& ChunkContext)
	{
		const TConstArrayView<FGreyfieldSquadFragment> Squads = ChunkContext.GetFragmentView<FGreyfieldSquadFragment>();
		const TConstArrayView<FTransformFragment> Transforms = ChunkContext.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMassMoveTargetFragment> MoveTargets = ChunkContext.GetFragmentView<FMassMoveTargetFragment>();

		for (int32 Index = 0; Index < ChunkContext.GetNumEntities(); ++Index)
		{
			if (Squads[Index].bIsSquadLeader)
			{
				FLeaderInfo Info;
				Info.Position = Transforms[Index].GetTransform().GetLocation();
				Info.FacingQuat = MoveTargets[Index].Forward.IsNearlyZero()
					? Transforms[Index].GetTransform().GetRotation()
					: MoveTargets[Index].Forward.ToOrientationQuat();
				Info.DesiredSpeed = MoveTargets[Index].DesiredSpeed.Get();
				LeaderInfoByHandle.Add(ChunkContext.GetEntity(Index), Info);
			}
		}
	});

	if (LeaderInfoByHandle.Num() == 0)
	{
		return;
	}

	// Pass 2: followers trail their leader's resolved position, offset by their formation slot.
	// Local avoidance (stock Mass Avoidance trait) handles not clumping/colliding along the way -
	// this processor only decides *where* each follower is currently trying to go.
	EntityQuery.ForEachEntityChunk(Context, [&LeaderInfoByHandle](FMassExecutionContext& ChunkContext)
	{
		const TConstArrayView<FGreyfieldSquadFragment> Squads = ChunkContext.GetFragmentView<FGreyfieldSquadFragment>();
		const TArrayView<FMassMoveTargetFragment> MoveTargets = ChunkContext.GetMutableFragmentView<FMassMoveTargetFragment>();

		for (int32 Index = 0; Index < ChunkContext.GetNumEntities(); ++Index)
		{
			if (Squads[Index].bIsSquadLeader)
			{
				continue;
			}

			const FLeaderInfo* LeaderInfo = LeaderInfoByHandle.Find(Squads[Index].LeaderEntity);
			if (!LeaderInfo)
			{
				continue;
			}

			const FVector2D& Offset = Squads[Index].FormationOffset;
			const FVector RotatedOffset = LeaderInfo->FacingQuat.RotateVector(FVector(Offset.X, Offset.Y, 0.f));

			MoveTargets[Index].Center = LeaderInfo->Position + RotatedOffset;
			MoveTargets[Index].Forward = LeaderInfo->FacingQuat.GetForwardVector();
			MoveTargets[Index].DesiredSpeed = FMassInt16Real(LeaderInfo->DesiredSpeed);
		}
	});
}
