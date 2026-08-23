// Operation Greyfield - default GameMode for skirmish matches.

#include "GreyfieldGameMode.h"
#include "GreyfieldPlayerController.h"
#include "GreyfieldRTSCameraPawn.h"
#include "GreyfieldHUD.h"
#include "GreyfieldPlayerState.h"

AGreyfieldGameMode::AGreyfieldGameMode()
{
	PlayerControllerClass = AGreyfieldPlayerController::StaticClass();
	DefaultPawnClass = AGreyfieldRTSCameraPawn::StaticClass();
	HUDClass = AGreyfieldHUD::StaticClass();
	PlayerStateClass = AGreyfieldPlayerState::StaticClass();
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
