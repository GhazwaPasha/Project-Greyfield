// Operation Greyfield - Phase 2 (Mass Entity player integration, 2026-08-29). The "High LOD"
// visual/selection proxy actor for a Mass-simulated unit: Mass's own representation system
// (UMassMovableVisualizationTrait::HighResTemplateActor, set on MEC_GreyfieldUnit_Leader/
// _Follower) spawns one of these automatically whenever an entity gets promoted to High LOD
// (near the camera or selected) and destroys it again once demoted - this class never spawns or
// destroys itself.
//
// Deliberately NOT AGreyfieldUnit: no GAS/ASC, no AIController/CharacterMovementComponent - the
// entity's Mass fragments are the source of truth for team/health/position, this actor is purely
// a thin, cheap visual+click-target skin over it. Combat and movement are entirely handled by the
// existing Mass processors (UGreyfieldSquadFormationProcessor, GreyfieldMassCombatProcessor);
// this class never simulates anything itself.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GreyfieldSelectableInterface.h"
#include "GreyfieldCommandableUnitInterface.h"
#include "Mass/EntityHandle.h"
#include "GreyfieldMassUnitVisual.generated.h"

UCLASS()
class PROJECTGREYFIELD_API AGreyfieldMassUnitVisual : public AActor, public IGreyfieldSelectableInterface, public IGreyfieldCommandableUnitInterface
{
	GENERATED_BODY()

public:
	AGreyfieldMassUnitVisual();

protected:
	virtual void BeginPlay() override;

public:
	// IGreyfieldSelectableInterface
	virtual void SetSelected_Implementation(bool bSelected);
	virtual bool IsSelected_Implementation() const;

	// IGreyfieldCommandableUnitInterface
	virtual EGreyfieldTeam GetGreyfieldTeam_Implementation() const;
	virtual void IssueMoveOrder_Implementation(const FVector& Destination);
	virtual void IssueAttackOrder_Implementation(AActor* Target);

	// Resolves this actor's own Mass entity handle via UMassActorSubsystem's actor-to-entity
	// lookup (populated by the representation system when it spawns/attaches this actor to an
	// entity). Invalid handle if called before that registration has happened, or after the
	// entity has already been destroyed/demoted. Exposed so GreyfieldPlayerController can batch
	// several selected AGreyfieldMassUnitVisual actors' handles into one real squad command
	// instead of ordering each independently.
	UFUNCTION(BlueprintPure, Category = "Greyfield|Mass")
	FMassEntityHandle GetEntityHandle() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Visual")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Visual")
	TObjectPtr<UStaticMeshComponent> SelectionRing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Selection")
	bool bIsSelected = false;
};
