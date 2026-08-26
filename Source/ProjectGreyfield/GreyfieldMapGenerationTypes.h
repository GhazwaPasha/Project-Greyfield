// Operation Greyfield - shared types for the procedural map generator. All Greyfield maps are
// generated fresh per match (AoE4-style), not hand-crafted like a classic Zero Hour map - this
// header defines the size tiers and the data the generator hands back to gameplay code.

#pragma once

#include "CoreMinimal.h"
#include "GreyfieldMapGenerationTypes.generated.h"

UENUM(BlueprintType)
enum class EGreyfieldMapSize : uint8
{
	Small2v2	UMETA(DisplayName = "Small (2v2)"),
	Medium3v3	UMETA(DisplayName = "Medium (3v3)"),
	Large4v4	UMETA(DisplayName = "Large (4v4)"),
	Gigantic	UMETA(DisplayName = "Gigantic")
};

// Tuning knobs per size tier. ComponentsPerSide drives heightmap resolution
// ((ComponentsPerSide * 63) + 1 verts/side, 63 quads/section being Unreal's own landscape
// default); NumPlayers is split evenly into two teams for point-symmetric spawn placement.
USTRUCT(BlueprintType)
struct FGreyfieldMapSizePreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Greyfield|MapGen")
	int32 ComponentsPerSide = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Greyfield|MapGen")
	int32 NumPlayers = 4;

	// Fraction of the map's half-extent at which player start points sit, out from center.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Greyfield|MapGen")
	float SpawnRadiusFraction = 0.62f;
};

USTRUCT(BlueprintType)
struct FGreyfieldPlayerSpawn
{
	GENERATED_BODY()

	// Map-local location (relative to map center, Z unset - caller should trace/offset onto
	// the generated terrain).
	UPROPERTY(BlueprintReadOnly, Category = "Greyfield|MapGen")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Greyfield|MapGen")
	int32 PlayerIndex = 0;

	// 0 or 1 - which side of the map's 180-degree symmetry this player belongs to.
	UPROPERTY(BlueprintReadOnly, Category = "Greyfield|MapGen")
	int32 TeamIndex = 0;
};
