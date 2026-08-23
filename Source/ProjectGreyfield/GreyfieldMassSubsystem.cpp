// Operation Greyfield - Phase 2 (Mass Entity backbone).

#include "GreyfieldMassSubsystem.h"
#include "GreyfieldMassFragments.h"
#include "MassEntitySubsystem.h"
#include "MassEntityManager.h"
#include "MassNavigationFragments.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntityConfigAsset.h"
#include "MassEntityTemplate.h"

DEFINE_LOG_CATEGORY_STATIC(LogGreyfieldMassTest, Log, All);

void UGreyfieldMassSubsystem::FormSquad(const TArray<FMassEntityHandle>& SquadEntities)
{
	if (SquadEntities.Num() == 0)
	{
		return;
	}

	UMassEntitySubsystem* MassEntitySubsystem = GetWorld() ? GetWorld()->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (!MassEntitySubsystem)
	{
		return;
	}
	FMassEntityManager& EntityManager = MassEntitySubsystem->GetMutableEntityManager();

	const FMassEntityHandle LeaderHandle = SquadEntities[0];
	static int32 NextSquadId = 1;
	const int32 SquadId = NextSquadId++;

	// Simple square-ish grid formation, matching AGreyfieldUnit's existing move-order spacing
	// intent (units shouldn't stack). Formation offsets are in the leader's local facing space.
	constexpr float Spacing = 150.f;
	const int32 Columns = FMath::CeilToInt(FMath::Sqrt((float)SquadEntities.Num()));

	for (int32 Index = 0; Index < SquadEntities.Num(); ++Index)
	{
		FGreyfieldSquadFragment* Squad = EntityManager.GetFragmentDataPtr<FGreyfieldSquadFragment>(SquadEntities[Index]);
		if (!Squad)
		{
			continue; // Entity wasn't spawned with the Greyfield Squad Member trait.
		}

		Squad->SquadId = SquadId;
		Squad->LeaderEntity = LeaderHandle;

		if (Index == 0)
		{
			Squad->bIsSquadLeader = true;
			Squad->FormationOffset = FVector2D::ZeroVector;
		}
		else
		{
			Squad->bIsSquadLeader = false;
			const int32 Row = Index / Columns;
			const int32 Col = Index % Columns;
			// Centered behind the leader rather than to one side.
			Squad->FormationOffset = FVector2D(-Spacing * (Row + 1), Spacing * (Col - Columns * 0.5f));
		}
	}
}

void UGreyfieldMassSubsystem::IssueSquadMoveOrder(const TArray<FMassEntityHandle>& SquadEntities, const FVector& Destination)
{
	if (SquadEntities.Num() == 0 || !GetWorld())
	{
		return;
	}

	UMassEntitySubsystem* MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!MassEntitySubsystem)
	{
		return;
	}
	FMassEntityManager& EntityManager = MassEntitySubsystem->GetMutableEntityManager();

	for (const FMassEntityHandle& Handle : SquadEntities)
	{
		const FGreyfieldSquadFragment* Squad = EntityManager.GetFragmentDataPtr<FGreyfieldSquadFragment>(Handle);
		if (!Squad || !Squad->bIsSquadLeader)
		{
			continue; // Only the leader gets a real order; followers pick it up via the formation processor.
		}

		FMassMoveTargetFragment* MoveTarget = EntityManager.GetFragmentDataPtr<FMassMoveTargetFragment>(Handle);
		if (!MoveTarget)
		{
			continue;
		}

		MoveTarget->CreateNewAction(EMassMovementAction::Move, *GetWorld());
		MoveTarget->Center = Destination;

		// DesiredSpeed defaults to 0 on a fresh action - without this the leader would path
		// correctly but never actually move. Pull it from the entity's own movement config
		// rather than hardcoding a value here.
		if (const FMassMovementParameters* MovementParams = EntityManager.GetConstSharedFragmentDataPtr<FMassMovementParameters>(Handle))
		{
			MoveTarget->DesiredSpeed = FMassInt16Real(MovementParams->DefaultDesiredSpeed);
		}
	}
}

void UGreyfieldMassSubsystem::SpawnAndTestSquad(UMassEntityConfigAsset* Config, int32 Count, const FVector& SpawnOrigin, const FVector& Destination)
{
	if (!Config || Count <= 0 || !GetWorld())
	{
		UE_LOG(LogGreyfieldMassTest, Error, TEXT("SpawnAndTestSquad: invalid Config/Count/World."));
		return;
	}

	UMassSpawnerSubsystem* SpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
	UMassEntitySubsystem* MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!SpawnerSubsystem || !MassEntitySubsystem)
	{
		UE_LOG(LogGreyfieldMassTest, Error, TEXT("SpawnAndTestSquad: Mass subsystems not available."));
		return;
	}
	FMassEntityManager& EntityManager = MassEntitySubsystem->GetMutableEntityManager();

	const FMassEntityTemplate& Template = Config->GetOrCreateEntityTemplate(*GetWorld());
	TArray<FMassEntityHandle> SpawnedEntities;
	SpawnerSubsystem->SpawnEntities(Template, static_cast<uint32>(Count), SpawnedEntities);

	// Spread entities in a small grid around SpawnOrigin so they don't all start stacked at one
	// point - SpawnEntities() itself doesn't take per-entity transforms.
	const int32 Columns = FMath::CeilToInt(FMath::Sqrt((float)SpawnedEntities.Num()));
	constexpr float SpawnSpacing = 150.f;
	for (int32 Index = 0; Index < SpawnedEntities.Num(); ++Index)
	{
		if (FTransformFragment* TransformFragment = EntityManager.GetFragmentDataPtr<FTransformFragment>(SpawnedEntities[Index]))
		{
			const int32 Row = Index / Columns;
			const int32 Col = Index % Columns;
			const FVector Offset(Row * SpawnSpacing, Col * SpawnSpacing, 0.f);
			TransformFragment->GetMutableTransform().SetLocation(SpawnOrigin + Offset);
		}
	}

	FormSquad(SpawnedEntities);
	IssueSquadMoveOrder(SpawnedEntities, Destination);

	UE_LOG(LogGreyfieldMassTest, Log, TEXT("SpawnAndTestSquad: spawned %d entities from '%s' at %s, squad formed, move order issued to %s."),
		SpawnedEntities.Num(), *Config->GetName(), *SpawnOrigin.ToString(), *Destination.ToString());
}
