// Operation Greyfield - placeholder unit. Self-contained visual (basic-shape cylinder body
// + a flat selection ring) so units spawned from this class look right with zero per-instance
// setup; real meshes replace BodyMesh in Phase 6. Carries a GAS AttributeSet for health/combat
// stats and a simple auto-attack loop; abilities/faction powers layer on top of this in a later
// pass rather than being built now.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GreyfieldSelectableInterface.h"
#include "GreyfieldCommandableUnitInterface.h"
#include "GreyfieldTeam.h"
#include "GreyfieldUnit.generated.h"

class UAbilitySystemComponent;
class UGreyfieldAttributeSet;

UCLASS()
class PROJECTGREYFIELD_API AGreyfieldUnit : public ACharacter, public IGreyfieldSelectableInterface, public IGreyfieldCommandableUnitInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGreyfieldUnit();

	virtual void Tick(float DeltaTime) override;

	virtual void SetSelected_Implementation(bool bSelected);
	virtual bool IsSelected_Implementation() const;

	// IGreyfieldCommandableUnitInterface
	virtual EGreyfieldTeam GetGreyfieldTeam_Implementation() const;
	virtual void IssueMoveOrder_Implementation(const FVector& Destination);
	virtual void IssueAttackOrder_Implementation(AActor* Target);

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Issues a NavMesh move order via this unit's AIController.
	UFUNCTION(BlueprintCallable, Category = "Greyfield|Movement")
	void MoveToLocation(const FVector& Destination);

	// Order to move within attack range of Target and auto-fight it once there.
	UFUNCTION(BlueprintCallable, Category = "Greyfield|Combat")
	void AttackMoveToTarget(AGreyfieldUnit* Target);

	// Order to move toward Destination, auto-engaging the nearest enemy that comes within
	// range along the way. Used by the AI director's attack waves; also usable for player
	// units later if attack-move-to-ground gets its own hotkey.
	UFUNCTION(BlueprintCallable, Category = "Greyfield|Combat")
	void AttackMoveToLocation(const FVector& Destination);

	UFUNCTION(BlueprintCallable, Category = "Greyfield|Combat")
	void ApplyDamage(float DamageAmount, AActor* DamageInstigator);

	UFUNCTION(BlueprintPure, Category = "Greyfield|Combat")
	bool IsDead() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Greyfield|Combat")
	EGreyfieldTeam Team = EGreyfieldTeam::Player;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Greyfield|Combat")
	float AttackRange = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Greyfield|Combat")
	float AttackInterval = 1.5f;

	// How far this unit sees. Enemy units outside every player unit's vision go hidden
	// (fog of war) - visual/rendering only for now, not a full shroud-of-unexplored-terrain
	// shader, and doesn't affect click-selectability yet.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Greyfield|Vision")
	float VisionRadius = 1200.f;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Selection")
	bool bIsSelected = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Visual")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Visual")
	TObjectPtr<UStaticMeshComponent> SelectionRing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Combat")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Combat")
	TObjectPtr<UGreyfieldAttributeSet> AttributeSet;

	UPROPERTY()
	TWeakObjectPtr<AGreyfieldUnit> AttackTarget;

	UPROPERTY()
	TWeakObjectPtr<class AGreyfieldBuilding> AttackTargetBuilding;

	float TimeSinceLastAttack = 0.f;

	// True while attack-moving to a location - enables the proximity aggro scan in Tick.
	bool bAggressive = false;

	void ScanForNearbyEnemy();
};
