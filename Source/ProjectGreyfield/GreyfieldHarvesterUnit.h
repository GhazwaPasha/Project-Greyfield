// Operation Greyfield - harvester: gather -> return -> deposit loop. Progress is tracked
// by distance-to-target each tick rather than a MoveCompleted delegate, which is simpler
// and good enough at placeholder scale; revisit if it gets flaky at higher unit counts.

#pragma once

#include "CoreMinimal.h"
#include "GreyfieldUnit.h"
#include "GreyfieldHarvesterUnit.generated.h"

class AGreyfieldResourceNode;
class AGreyfieldBuilding;

UENUM()
enum class EGreyfieldHarvestState : uint8
{
	Idle,
	MovingToResource,
	Harvesting,
	ReturningToDeposit,
	Depositing
};

UCLASS()
class PROJECTGREYFIELD_API AGreyfieldHarvesterUnit : public AGreyfieldUnit
{
	GENERATED_BODY()

public:
	AGreyfieldHarvesterUnit();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Greyfield|Economy")
	void OrderHarvest(AGreyfieldResourceNode* Node);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Greyfield|Economy")
	int32 CarryCapacity = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Greyfield|Economy")
	float HarvestRate = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Greyfield|Economy")
	float GatherAcceptanceRadius = 150.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Economy")
	int32 CarriedAmount = 0;

	UPROPERTY()
	TWeakObjectPtr<AGreyfieldResourceNode> TargetNode;

	UPROPERTY()
	TWeakObjectPtr<AGreyfieldBuilding> DepositTarget;

	EGreyfieldHarvestState HarvestState = EGreyfieldHarvestState::Idle;

private:
	AGreyfieldBuilding* FindNearestDepositPoint() const;
};
