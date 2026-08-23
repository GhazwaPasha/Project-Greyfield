// Operation Greyfield - shared DataTable row schemas.
// Kept in one header for Phase 0 scaffolding; split out if it grows unwieldy.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "GreyfieldDataTypes.generated.h"

// A single row in the units DataTable: one row per unit type in the game.
USTRUCT(BlueprintType)
struct FGreyfieldUnitDataRow : public FTableRowBase
{
	GENERATED_BODY()

	// Display name shown in the command card / unit info panel.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	// Blueprint class to spawn for this unit. Left unset until the unit exists.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	TSoftClassPtr<AActor> UnitClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float MoveSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float AttackDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float AttackRange = 0.f;

	// Primary resource cost to produce this unit.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int32 ResourceCost = 0;

	// Seconds to produce this unit from a build queue.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	float ProductionTime = 0.f;

	// Gameplay tag identifying the building(s) required before this unit can be queued.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	FGameplayTag RequiredPrerequisite;
};

// A single row in the buildings DataTable: one row per structure type in the game.
USTRUCT(BlueprintType)
struct FGreyfieldBuildingDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	TSoftClassPtr<AActor> BuildingClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float MaxHealth = 500.f;

	// Primary resource cost to construct this building.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int32 ResourceCost = 0;

	// Seconds to complete construction.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	float ConstructionTime = 0.f;

	// Secondary resource (power-style) this building supplies. 0 or negative means it consumes instead.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int32 SecondaryResourceSupplied = 0;

	// Gameplay tag identifying the building(s) required before this one can be queued.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	FGameplayTag RequiredPrerequisite;
};
