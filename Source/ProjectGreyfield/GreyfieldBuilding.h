// Operation Greyfield - placeable structure: construction timer, health, and a
// data-driven unit build queue read from DT_Units.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GreyfieldSelectableInterface.h"
#include "GreyfieldBuilding.generated.h"

class UDataTable;

USTRUCT()
struct FGreyfieldQueuedUnit
{
	GENERATED_BODY()

	UPROPERTY()
	FName UnitRowName;

	UPROPERTY()
	float RemainingTime = 0.f;
};

UCLASS()
class PROJECTGREYFIELD_API AGreyfieldBuilding : public AActor, public IGreyfieldSelectableInterface
{
	GENERATED_BODY()

public:
	AGreyfieldBuilding();

	virtual void Tick(float DeltaTime) override;

	virtual void SetSelected_Implementation(bool bSelected);
	virtual bool IsSelected_Implementation() const;

	// Names + remaining seconds of everything currently queued, front item first.
	UFUNCTION(BlueprintPure, Category = "Greyfield|Building")
	TArray<FString> GetQueueSummary() const;

	// Spends the row's ResourceCost from the local player's stockpile (fails without
	// queueing if unaffordable) and appends it to the build queue.
	UFUNCTION(BlueprintCallable, Category = "Greyfield|Building")
	bool QueueUnit(FName UnitRowName);

	UFUNCTION(BlueprintPure, Category = "Greyfield|Building")
	bool IsUnderConstruction() const { return bUnderConstruction; }

	UFUNCTION(BlueprintPure, Category = "Greyfield|Building")
	float GetConstructionFraction() const;

	// Skips straight to fully-built. Used for the starting HQ so testing doesn't require
	// waiting out a construction timer on a building nobody placed.
	UFUNCTION(BlueprintCallable, Category = "Greyfield|Building")
	void CompleteConstructionInstantly();

	// Reduces health and destroys the building at 0. If this is the player's HQ
	// (bDepositPoint), also ends the match via the GameMode.
	UFUNCTION(BlueprintCallable, Category = "Greyfield|Building")
	void ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintPure, Category = "Greyfield|Building")
	bool IsDestroyed() const { return CurrentHealth <= 0.f; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Greyfield|Building")
	bool bDepositPoint = false;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Greyfield|Building")
	float MaxHealth = 500.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Building")
	float CurrentHealth = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Greyfield|Building")
	float ConstructionDuration = 5.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Building")
	bool bUnderConstruction = true;

	float ConstructionTimeRemaining = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Greyfield|Building")
	TObjectPtr<UDataTable> UnitDataTable;

	UPROPERTY()
	TArray<FGreyfieldQueuedUnit> BuildQueue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Visual")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Visual")
	TObjectPtr<UStaticMeshComponent> SelectionRing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Selection")
	bool bIsSelected = false;

private:
	void ProcessBuildQueue(float DeltaTime);
};
