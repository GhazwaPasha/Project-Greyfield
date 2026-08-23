// Operation Greyfield - player resource stockpiles.

#include "GreyfieldPlayerState.h"

void AGreyfieldPlayerState::AddPrimaryResource(int32 Amount)
{
	PrimaryResource = FMath::Max(0, PrimaryResource + Amount);
}

bool AGreyfieldPlayerState::SpendPrimaryResource(int32 Amount)
{
	if (Amount <= 0)
	{
		return true;
	}
	if (PrimaryResource < Amount)
	{
		return false;
	}
	PrimaryResource -= Amount;
	return true;
}

void AGreyfieldPlayerState::AddSecondaryResource(int32 Amount)
{
	SecondaryResource = FMath::Max(0, SecondaryResource + Amount);
}
