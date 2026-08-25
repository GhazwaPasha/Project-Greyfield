// Operation Greyfield - GameMode variant for procedurally-generated levels (Map_Small2v2 etc).
// Identical to AGreyfieldGameMode except it turns bGenerateProceduralMap on by default, so the
// map builds in InitGame before any player spawns. NewMap (the hand-built vertical slice level)
// keeps using the base AGreyfieldGameMode, untouched by this.

#pragma once

#include "CoreMinimal.h"
#include "GreyfieldGameMode.h"
#include "GreyfieldGameMode_Procedural.generated.h"

UCLASS()
class PROJECTGREYFIELD_API AGreyfieldGameMode_Procedural : public AGreyfieldGameMode
{
	GENERATED_BODY()

public:
	AGreyfieldGameMode_Procedural();
};
