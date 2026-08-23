// Operation Greyfield - Phase 2 (Mass Entity backbone). DEV/TEST ONLY - not part of the real
// game. A placeable actor that spawns a squad of Mass entities and issues a move order on
// BeginPlay, to prove FormSquad/IssueSquadMoveOrder/UGreyfieldSquadFormationProcessor actually
// work in PIE before the real player-order UI is wired up to Mass entities. Safe to delete once
// that real integration exists - this is scaffolding, not a permanent gameplay actor.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GreyfieldMassSquadTestTrigger.generated.h"

class UMassEntityConfigAsset;

UCLASS()
class PROJECTGREYFIELD_API AGreyfieldMassSquadTestTrigger : public AActor
{
	GENERATED_BODY()

public:
	AGreyfieldMassSquadTestTrigger();

	UPROPERTY(EditAnywhere, Category = "Greyfield|Mass|Test")
	TObjectPtr<UMassEntityConfigAsset> EntityConfig;

	UPROPERTY(EditAnywhere, Category = "Greyfield|Mass|Test")
	int32 SquadCount = 9;

	// World-space offset from this actor's own location where the squad spawns.
	UPROPERTY(EditAnywhere, Category = "Greyfield|Mass|Test")
	FVector SpawnOriginOffset = FVector::ZeroVector;

	// World-space offset from this actor's own location the squad is ordered to move to.
	UPROPERTY(EditAnywhere, Category = "Greyfield|Mass|Test")
	FVector DestinationOffset = FVector(3000.f, 0.f, 0.f);

protected:
	virtual void BeginPlay() override;

	// Without a root component, AActor has no meaningful transform - GetActorLocation() silently
	// returns zero regardless of where the actor was placed. Bit us once already (2026-08-24):
	// the first test run spawned/ordered relative to (0,0,0) instead of this actor's placed
	// location.
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;
};
