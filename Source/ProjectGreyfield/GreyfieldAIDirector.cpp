// Operation Greyfield - skirmish AI opponent.

#include "GreyfieldAIDirector.h"
#include "GreyfieldUnit.h"
#include "GreyfieldBuilding.h"
#include "EngineUtils.h"
#include "Engine/World.h"

AGreyfieldAIDirector::AGreyfieldAIDirector()
{
	PrimaryActorTick.bCanEverTick = true;
	UnitClassToSpawn = AGreyfieldUnit::StaticClass();
}

void AGreyfieldAIDirector::BeginPlay()
{
	Super::BeginPlay();
	TimeSinceLastSpawn = 0.f;
}

void AGreyfieldAIDirector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeSinceLastSpawn += DeltaTime;
	if (TimeSinceLastSpawn >= UnitSpawnInterval)
	{
		TimeSinceLastSpawn = 0.f;
		SpawnEnemyUnit();
	}

	if (Army.Num() >= UnitsPerWave)
	{
		LaunchAttack();
	}
}

void AGreyfieldAIDirector::SpawnEnemyUnit()
{
	if (!UnitClassToSpawn || !GetWorld())
	{
		return;
	}

	// Small scatter around the director so a wave doesn't spawn stacked on one point.
	const FVector Offset(FMath::RandRange(-200.f, 200.f), FMath::RandRange(-200.f, 200.f), 0.f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AGreyfieldUnit* NewUnit = GetWorld()->SpawnActor<AGreyfieldUnit>(UnitClassToSpawn, GetActorLocation() + Offset, GetActorRotation(), SpawnParams);
	if (NewUnit)
	{
		NewUnit->Team = EGreyfieldTeam::Enemy;
		Army.Add(NewUnit);
	}
}

AActor* AGreyfieldAIDirector::FindPlayerHQ() const
{
	for (TActorIterator<AGreyfieldBuilding> It(GetWorld()); It; ++It)
	{
		if (It->bDepositPoint)
		{
			return *It;
		}
	}
	return nullptr;
}

void AGreyfieldAIDirector::LaunchAttack()
{
	AActor* TargetHQ = FindPlayerHQ();
	const FVector AttackDestination = TargetHQ ? TargetHQ->GetActorLocation() : GetActorLocation();

	for (const TWeakObjectPtr<AGreyfieldUnit>& Weak : Army)
	{
		if (AGreyfieldUnit* Unit = Weak.Get())
		{
			if (!Unit->IsDead())
			{
				Unit->AttackMoveToLocation(AttackDestination);
			}
		}
	}

	// Wave is committed - start building the next one from scratch rather than
	// re-triggering the attack order every tick while this wave is still alive.
	Army.Reset();
}
