// Operation Greyfield - default GameMode for skirmish matches.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GreyfieldMapGenerationTypes.h"
#include "GreyfieldGameMode.generated.h"

UCLASS()
class PROJECTGREYFIELD_API AGreyfieldGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AGreyfieldGameMode();

	// Called when the player's deposit-point HQ is destroyed. Idempotent.
	UFUNCTION(BlueprintCallable, Category = "Greyfield|Match")
	void OnPlayerHQDestroyed();

	UFUNCTION(BlueprintPure, Category = "Greyfield|Match")
	bool IsMatchOver() const { return bMatchOver; }

protected:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Match")
	bool bMatchOver = false;

	// Off by default so the existing hand-built NewMap level is untouched -
	// AGreyfieldGameMode_Procedural turns this on for procedurally-generated levels.
	// GenerateMap runs from InitGame (before any player login/spawn), which is what actually
	// fixes the spawn-ordering race the old dev-trigger-actor approach had: FindPlayerStart used
	// to run before the trigger's BeginPlay had generated any PlayerStart, so the pawn fell back
	// to spawning at world origin - which can land it embedded inside solid landscape geometry
	// if the terrain there happens to be elevated (black screen, confirmed 2026-08-25).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Greyfield|MapGen")
	bool bGenerateProceduralMap = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Greyfield|MapGen")
	EGreyfieldMapSize ProceduralMapSize = EGreyfieldMapSize::Small2v2;

	// 0 = a random seed every match - the real "regenerates every time" behavior.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Greyfield|MapGen")
	int32 ProceduralMapSeed = 0;
};
