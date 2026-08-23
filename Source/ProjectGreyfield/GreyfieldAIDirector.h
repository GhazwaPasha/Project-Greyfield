// Operation Greyfield - skirmish AI opponent. No Behavior Tree/EQS here: the MCP toolset
// can only inspect BT assets, not author them, and there's no way to build one visually
// without editor access this session doesn't have. Same design intent as the doc's
// "build-order script: economy -> army -> attack timer" - implemented as a plain C++
// timer loop instead. Swap in a real BT once BT assets can actually be created/edited.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GreyfieldAIDirector.generated.h"

class AGreyfieldUnit;

UCLASS()
class PROJECTGREYFIELD_API AGreyfieldAIDirector : public AActor
{
	GENERATED_BODY()

public:
	AGreyfieldAIDirector();

	virtual void Tick(float DeltaTime) override;

	// Seconds between each unit spawn. Difficulty lever: lower = faster buildup.
	UPROPERTY(EditAnywhere, Category = "Greyfield|AI")
	float UnitSpawnInterval = 8.f;

	// Army size that triggers an attack wave.
	UPROPERTY(EditAnywhere, Category = "Greyfield|AI")
	int32 UnitsPerWave = 3;

	UPROPERTY(EditAnywhere, Category = "Greyfield|AI")
	TSubclassOf<AGreyfieldUnit> UnitClassToSpawn;

protected:
	virtual void BeginPlay() override;

private:
	void SpawnEnemyUnit();
	void LaunchAttack();
	AActor* FindPlayerHQ() const;

	float TimeSinceLastSpawn = 0.f;

	UPROPERTY()
	TArray<TWeakObjectPtr<AGreyfieldUnit>> Army;
};
