// Operation Greyfield - the procedural map generator. Builds a heightmap-sculpted Landscape
// (MW Landscape Auto Material applied, no manual paint layers needed) plus a set of
// point-symmetric player start locations, sized per EGreyfieldMapSize. Same seed always
// reproduces the same map; seed 0 picks a random one - this is the "regenerate a new map every
// match, like AoE4" system the fixed hand-crafted Zero-Hour-style level (NewMap) is not.
//
// KNOWN GAP (see PROJECT_STATUS.md): GenerateMap is WITH_EDITOR-gated. ALandscapeProxy::Import(),
// the only engine API that builds real landscape components from raw height data, lives entirely
// inside Landscape's own WITH_EDITOR block and is compiled out of Shipping/packaged builds. It
// works from any Editor-context executable - including a normal PIE session, which is all this
// project currently ships through - but not from a cooked game. Packaging will need this ported
// to a runtime-safe terrain representation (e.g. a ProceduralMeshComponent heightfield) or a
// pre-bake-per-seed step; not done here.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GreyfieldMapGenerationTypes.h"
#include "GreyfieldMapGenerationSubsystem.generated.h"

class ALandscape;

UCLASS()
class PROJECTGREYFIELD_API UGreyfieldMapGenerationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Generates a brand-new map into this subsystem's world: spawns and sculpts an ALandscape,
	// applies the MW Auto Material, and spawns a APlayerStart per generated slot. Returns the
	// new Landscape (nullptr on failure, including the WITH_EDITOR gap above).
	UFUNCTION(BlueprintCallable, Category = "Greyfield|MapGen")
	ALandscape* GenerateMap(EGreyfieldMapSize MapSize, int32 Seed);

	UFUNCTION(BlueprintPure, Category = "Greyfield|MapGen")
	static FGreyfieldMapSizePreset GetPresetForSize(EGreyfieldMapSize MapSize);

	// Spawn points from the most recent GenerateMap call, for anything that wants to place
	// starting bases/units without re-deriving the layout.
	UPROPERTY(BlueprintReadOnly, Category = "Greyfield|MapGen")
	TArray<FGreyfieldPlayerSpawn> LastGeneratedSpawns;

private:
	// Team-symmetric spawn ring: half the players on a "north" arc, the other half at each of
	// those points rotated exactly 180 degrees - guarantees both teams get point-mirrored
	// (equally fair) starting terrain once BuildHeightmap mirrors the same way.
	TArray<FGreyfieldPlayerSpawn> BuildSpawnPoints(const FGreyfieldMapSizePreset& Preset, int32 SizeVerts, float QuadWorldSize) const;

	// Layered Perlin (fBm) heightmap: shaped to bias toward buildable plains with rolling hills,
	// flattened into a buildable pad under every spawn, and mirrored under 180-degree point
	// symmetry about the map center so the two teams' terrain is provably fair, not just
	// statistically similar. Returns raw landscape height samples (SizeVerts x SizeVerts).
	TArray<uint16> BuildHeightmap(int32 SizeVerts, int32 Seed, const TArray<FGreyfieldPlayerSpawn>& Spawns, float QuadWorldSize, float ActorScaleZ) const;
};
