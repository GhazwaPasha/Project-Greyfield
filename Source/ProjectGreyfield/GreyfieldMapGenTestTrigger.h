// Operation Greyfield - DEV/TEST ONLY, not part of the real game. A placeable actor that calls
// UGreyfieldMapGenerationSubsystem::GenerateMap on BeginPlay, so a level containing just this
// actor proves the procedural map generator (heightmap + MW Auto Material + symmetric spawns)
// actually works in PIE before any real map-select/lobby flow exists to trigger it for real.
// Safe to delete once that real flow exists - this is scaffolding, matching
// AGreyfieldMassSquadTestTrigger's role for Phase 2.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GreyfieldMapGenerationTypes.h"
#include "GreyfieldMapGenTestTrigger.generated.h"

UCLASS()
class PROJECTGREYFIELD_API AGreyfieldMapGenTestTrigger : public AActor
{
	GENERATED_BODY()

public:
	AGreyfieldMapGenTestTrigger();

	UPROPERTY(EditAnywhere, Category = "Greyfield|MapGen|Test")
	EGreyfieldMapSize MapSize = EGreyfieldMapSize::Small2v2;

	// 0 = pick a random seed each run (the real "regenerates every match" behavior). Set a
	// specific value to reproduce the same map while debugging.
	UPROPERTY(EditAnywhere, Category = "Greyfield|MapGen|Test")
	int32 Seed = 0;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;
};
