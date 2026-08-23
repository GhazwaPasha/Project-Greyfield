// Operation Greyfield - a harvestable resource deposit. Depletes and destroys itself
// when drained; harvester units call Harvest() rather than reading RemainingAmount
// directly so depletion can't race across multiple harvesters in one tick.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GreyfieldResourceNode.generated.h"

UCLASS()
class PROJECTGREYFIELD_API AGreyfieldResourceNode : public AActor
{
	GENERATED_BODY()

public:
	AGreyfieldResourceNode();

	// Removes up to RequestedAmount from the node and returns how much was actually
	// taken (may be less than requested near depletion). Destroys the node when emptied.
	UFUNCTION(BlueprintCallable, Category = "Greyfield|Economy")
	int32 Harvest(int32 RequestedAmount);

	UFUNCTION(BlueprintPure, Category = "Greyfield|Economy")
	bool IsDepleted() const { return RemainingAmount <= 0; }

	UFUNCTION(BlueprintPure, Category = "Greyfield|Economy")
	int32 GetRemainingAmount() const { return RemainingAmount; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Greyfield|Economy")
	int32 RemainingAmount = 1000;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Visual")
	TObjectPtr<UStaticMeshComponent> BodyMesh;
};
