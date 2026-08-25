// Operation Greyfield - see GreyfieldMapGenTestTrigger.h.

#include "GreyfieldMapGenTestTrigger.h"
#include "GreyfieldMapGenerationSubsystem.h"

AGreyfieldMapGenTestTrigger::AGreyfieldMapGenTestTrigger()
{
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
}

void AGreyfieldMapGenTestTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (UGreyfieldMapGenerationSubsystem* MapGen = GetWorld()->GetSubsystem<UGreyfieldMapGenerationSubsystem>())
	{
		MapGen->GenerateMap(MapSize, Seed);
	}
}
