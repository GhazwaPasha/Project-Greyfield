// Operation Greyfield - default GameMode for skirmish matches.

#include "GreyfieldGameMode.h"
#include "GreyfieldPlayerController.h"
#include "GreyfieldRTSCameraPawn.h"
#include "GreyfieldHUD.h"
#include "GreyfieldPlayerState.h"
#include "GreyfieldMapGenerationSubsystem.h"

AGreyfieldGameMode::AGreyfieldGameMode()
{
	PlayerControllerClass = AGreyfieldPlayerController::StaticClass();
	DefaultPawnClass = AGreyfieldRTSCameraPawn::StaticClass();
	HUDClass = AGreyfieldHUD::StaticClass();
	PlayerStateClass = AGreyfieldPlayerState::StaticClass();
}

void AGreyfieldGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void AGreyfieldGameMode::StartPlay()
{
	// 2026-08-26: moved here from InitGame while investigating the still-open Lit-mode-black bug
	// on Map_Small2v2 (PROJECT_STATUS.md, "root cause #4"). InitGame runs before ANY level actor's
	// BeginPlay and before UWorld::BeginPlay() itself (confirmed by reading World.cpp:
	// AuthorityGameMode->InitGame() fires inside InitializeActorsForPlay, before that function's
	// own Level->RouteActorInitialize() call which is what dispatches BeginPlay to pre-placed
	// actors - so a landscape spawned in InitGame exists before the Sun/SkyLight/SkyAtmosphere/
	// PostProcessVolume actors placed in the level have had their own BeginPlay run, and before
	// any world subsystem's OnWorldBeginPlay fires). Every other level's landscape (including the
	// MW Auto Material plugin's own working reference maps) instead loads as part of normal level
	// load, long before InitGame runs at all - a completely different point in world startup.
	// Exhaustive property-level comparison against that reference map (lighting, material,
	// weightmaps, RVT, Nanite - see PROJECT_STATUS.md) found no difference, so this timing is the
	// most likely remaining structural cause. StartPlay is called from inside UWorld::BeginPlay(),
	// after Level->RouteActorInitialize() and after every WorldSubsystem's OnWorldBeginPlay() have
	// already run - the closest available hook to "just like a normal level load" while still
	// running BEFORE Super::StartPlay() below triggers player login/FindPlayerStart, which is what
	// actually matters for not regressing the original spawn-ordering race fix.
	if (bGenerateProceduralMap)
	{
		if (UGreyfieldMapGenerationSubsystem* MapGen = GetWorld()->GetSubsystem<UGreyfieldMapGenerationSubsystem>())
		{
			MapGen->GenerateMap(ProceduralMapSize, ProceduralMapSeed);
		}
	}

	Super::StartPlay();
}

void AGreyfieldGameMode::OnPlayerHQDestroyed()
{
	if (bMatchOver)
	{
		return;
	}
	bMatchOver = true;
	UE_LOG(LogTemp, Warning, TEXT("Greyfield: DEFEAT - player HQ destroyed"));
}
