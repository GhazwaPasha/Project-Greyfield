// Operation Greyfield - Phase 2 (Mass Entity backbone). DEV/TEST ONLY, see header.

#include "GreyfieldMassSquadTestTrigger.h"
#include "GreyfieldMassSubsystem.h"

AGreyfieldMassSquadTestTrigger::AGreyfieldMassSquadTestTrigger()
{
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void AGreyfieldMassSquadTestTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (UGreyfieldMassSubsystem* MassSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UGreyfieldMassSubsystem>() : nullptr)
	{
		const FVector Origin = GetActorLocation() + SpawnOriginOffset;
		const FVector Destination = GetActorLocation() + DestinationOffset;
		MassSubsystem->SpawnAndTestSquad(EntityConfig, SquadCount, Origin, Destination);
	}
}
