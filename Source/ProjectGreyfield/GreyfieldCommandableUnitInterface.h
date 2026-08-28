// Operation Greyfield - Phase 2 (Mass Entity player integration, 2026-08-29). Implemented by
// anything the player can issue move/attack orders to as a selected combat unit - both the
// existing Actor-based AGreyfieldUnit and the new Mass-Entity-backed AGreyfieldMassUnitVisual.
// Kept separate from IGreyfieldSelectableInterface, which also covers non-orderable selectables
// like AGreyfieldBuilding (a building is selectable but never gets a move/attack order).
//
// GreyfieldPlayerController dispatches through this interface instead of casting to a concrete
// unit class, so box-select/move-order/attack-order all work uniformly across both unit
// representations without per-class special-casing spreading through the controller.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GreyfieldTeam.h"
#include "GreyfieldCommandableUnitInterface.generated.h"

UINTERFACE(BlueprintType)
class PROJECTGREYFIELD_API UGreyfieldCommandableUnitInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECTGREYFIELD_API IGreyfieldCommandableUnitInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Greyfield|Command")
	EGreyfieldTeam GetGreyfieldTeam() const;

	// Independent move order for this unit alone. For a Mass-Entity-backed unit this is a
	// solo squad-of-one order (see AGreyfieldMassUnitVisual) - when commanding several
	// Mass-backed units together as one group, GreyfieldPlayerController bypasses this and
	// calls UGreyfieldMassSubsystem::FormSquad/IssueSquadMoveOrder directly on the whole group
	// instead, so the group gets real squad formation rather than N independent solo squads.
	UFUNCTION(BlueprintNativeEvent, Category = "Greyfield|Command")
	void IssueMoveOrder(const FVector& Destination);

	// Move within attack range of Target and auto-fight it once there.
	UFUNCTION(BlueprintNativeEvent, Category = "Greyfield|Command")
	void IssueAttackOrder(AActor* Target);
};
