// Operation Greyfield - default GameMode for skirmish matches.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Match")
	bool bMatchOver = false;
};
