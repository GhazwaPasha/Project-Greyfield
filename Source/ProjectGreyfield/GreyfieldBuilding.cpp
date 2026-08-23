// Operation Greyfield - placeable structure.

#include "GreyfieldBuilding.h"
#include "GreyfieldPlayerState.h"
#include "GreyfieldGameMode.h"
#include "GreyfieldDataTypes.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AGreyfieldBuilding::AGreyfieldBuilding()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	RootComponent = BodyMesh;
	BodyMesh->SetRelativeScale3D(FVector(3.f, 3.f, 2.f));
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BodyMesh->SetCollisionResponseToAllChannels(ECR_Block);
	if (CubeMeshFinder.Succeeded())
	{
		BodyMesh->SetStaticMesh(CubeMeshFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> UnitTableFinder(TEXT("/Game/Data/DT_Units.DT_Units"));
	if (UnitTableFinder.Succeeded())
	{
		UnitDataTable = UnitTableFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> RingMeshFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	SelectionRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectionRing"));
	SelectionRing->SetupAttachment(BodyMesh);
	SelectionRing->SetRelativeLocation(FVector(0.f, 0.f, -49.f));
	SelectionRing->SetRelativeScale3D(FVector(1.15f, 1.15f, 0.01f));
	SelectionRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SelectionRing->SetVisibility(false);
	if (RingMeshFinder.Succeeded())
	{
		SelectionRing->SetStaticMesh(RingMeshFinder.Object);
	}
}

void AGreyfieldBuilding::SetSelected_Implementation(bool bSelected)
{
	bIsSelected = bSelected;
	if (SelectionRing)
	{
		SelectionRing->SetVisibility(bSelected);
	}
}

bool AGreyfieldBuilding::IsSelected_Implementation() const
{
	return bIsSelected;
}

TArray<FString> AGreyfieldBuilding::GetQueueSummary() const
{
	TArray<FString> Summary;
	for (const FGreyfieldQueuedUnit& Item : BuildQueue)
	{
		Summary.Add(FString::Printf(TEXT("%s (%.0fs)"), *Item.UnitRowName.ToString(), Item.RemainingTime));
	}
	return Summary;
}

void AGreyfieldBuilding::BeginPlay()
{
	Super::BeginPlay();

	bUnderConstruction = true;
	ConstructionTimeRemaining = ConstructionDuration;
	CurrentHealth = FMath::Max(1.f, MaxHealth * 0.1f);
}

void AGreyfieldBuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bUnderConstruction)
	{
		ConstructionTimeRemaining = FMath::Max(0.f, ConstructionTimeRemaining - DeltaTime);
		CurrentHealth = FMath::Lerp(MaxHealth * 0.1f, MaxHealth, GetConstructionFraction());

		if (ConstructionTimeRemaining <= 0.f)
		{
			bUnderConstruction = false;
			CurrentHealth = MaxHealth;
		}
		return;
	}

	ProcessBuildQueue(DeltaTime);
}

float AGreyfieldBuilding::GetConstructionFraction() const
{
	if (ConstructionDuration <= 0.f)
	{
		return 1.f;
	}
	return 1.f - (ConstructionTimeRemaining / ConstructionDuration);
}

void AGreyfieldBuilding::CompleteConstructionInstantly()
{
	bUnderConstruction = false;
	ConstructionTimeRemaining = 0.f;
	CurrentHealth = MaxHealth;
}

void AGreyfieldBuilding::ApplyDamage(float DamageAmount)
{
	if (bUnderConstruction || DamageAmount <= 0.f || IsDestroyed())
	{
		return;
	}

	CurrentHealth = FMath::Max(0.f, CurrentHealth - DamageAmount);

	if (CurrentHealth <= 0.f)
	{
		if (bDepositPoint)
		{
			if (AGreyfieldGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AGreyfieldGameMode>() : nullptr)
			{
				GM->OnPlayerHQDestroyed();
			}
		}
		Destroy();
	}
}

bool AGreyfieldBuilding::QueueUnit(FName UnitRowName)
{
	if (bUnderConstruction || !UnitDataTable)
	{
		return false;
	}

	static const FString ContextString(TEXT("GreyfieldBuilding.QueueUnit"));
	const FGreyfieldUnitDataRow* Row = UnitDataTable->FindRow<FGreyfieldUnitDataRow>(UnitRowName, ContextString);
	if (!Row)
	{
		return false;
	}

	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	AGreyfieldPlayerState* PS = PC ? Cast<AGreyfieldPlayerState>(PC->PlayerState) : nullptr;
	if (!PS || !PS->SpendPrimaryResource(Row->ResourceCost))
	{
		return false;
	}

	FGreyfieldQueuedUnit Queued;
	Queued.UnitRowName = UnitRowName;
	Queued.RemainingTime = Row->ProductionTime;
	BuildQueue.Add(Queued);
	return true;
}

void AGreyfieldBuilding::ProcessBuildQueue(float DeltaTime)
{
	if (BuildQueue.Num() == 0)
	{
		return;
	}

	FGreyfieldQueuedUnit& Front = BuildQueue[0];
	Front.RemainingTime -= DeltaTime;
	if (Front.RemainingTime > 0.f)
	{
		return;
	}

	static const FString ContextString(TEXT("GreyfieldBuilding.ProcessBuildQueue"));
	const FGreyfieldUnitDataRow* Row = UnitDataTable ? UnitDataTable->FindRow<FGreyfieldUnitDataRow>(Front.UnitRowName, ContextString) : nullptr;
	BuildQueue.RemoveAt(0);

	if (!Row || Row->UnitClass.IsNull())
	{
		// No spawnable class wired up on this row yet - the queue still consumed
		// the resource and time; nothing to spawn until Phase 4 fills in real units.
		return;
	}

	UClass* UnitClass = Row->UnitClass.LoadSynchronous();
	if (!UnitClass)
	{
		return;
	}

	const FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 300.f;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	GetWorld()->SpawnActor<AActor>(UnitClass, SpawnLocation, GetActorRotation(), SpawnParams);
}
