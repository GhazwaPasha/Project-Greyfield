// Operation Greyfield - shared team enum, split out from GreyfieldUnit.h (2026-08-29) so it can
// be included by headers that need EGreyfieldTeam but shouldn't drag in the full AGreyfieldUnit
// ACharacter class - notably GreyfieldCommandableUnitInterface.h and GreyfieldMassFragments.h,
// which would otherwise form an include cycle back through GreyfieldUnit.h.

#pragma once

#include "CoreMinimal.h"
#include "GreyfieldTeam.generated.h"

UENUM(BlueprintType)
enum class EGreyfieldTeam : uint8
{
	Player,
	Enemy
};
