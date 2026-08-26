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

	// Runs before any player login/spawn - see the bGenerateProceduralMap comment in the header
	// for why this has to happen here and not from a placed actor's BeginPlay.
	if (bGenerateProceduralMap)
	{
		if (UGreyfieldMapGenerationSubsystem* MapGen = GetWorld()->GetSubsystem<UGreyfieldMapGenerationSubsystem>())
		{
			MapGen->GenerateMap(ProceduralMapSize, ProceduralMapSeed);
		}
	}
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
