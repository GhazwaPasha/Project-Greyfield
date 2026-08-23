// Operation Greyfield - harvester gather/return/deposit loop.

#include "GreyfieldHarvesterUnit.h"
#include "GreyfieldResourceNode.h"
#include "GreyfieldBuilding.h"
#include "GreyfieldPlayerState.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

AGreyfieldHarvesterUnit::AGreyfieldHarvesterUnit()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGreyfieldHarvesterUnit::OrderHarvest(AGreyfieldResourceNode* Node)
{
	if (!Node)
	{
		return;
	}
	TargetNode = Node;
	MoveToLocation(Node->GetActorLocation());
	HarvestState = EGreyfieldHarvestState::MovingToResource;
}

AGreyfieldBuilding* AGreyfieldHarvesterUnit::FindNearestDepositPoint() const
{
	AGreyfieldBuilding* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();

	for (TActorIterator<AGreyfieldBuilding> It(GetWorld()); It; ++It)
	{
		AGreyfieldBuilding* Building = *It;
		if (!Building->bDepositPoint || Building->IsUnderConstruction())
		{
			continue;
		}
		const float DistSq = FVector::DistSquared(GetActorLocation(), Building->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Building;
		}
	}
	return Best;
}

void AGreyfieldHarvesterUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (HarvestState)
	{
	case EGreyfieldHarvestState::MovingToResource:
	{
		AGreyfieldResourceNode* Node = TargetNode.Get();
		if (!Node || Node->IsDepleted())
		{
			HarvestState = EGreyfieldHarvestState::Idle;
			break;
		}
		if (FVector::Dist(GetActorLocation(), Node->GetActorLocation()) <= GatherAcceptanceRadius)
		{
			HarvestState = EGreyfieldHarvestState::Harvesting;
		}
		break;
	}
	case EGreyfieldHarvestState::Harvesting:
	{
		AGreyfieldResourceNode* Node = TargetNode.Get();
		if (!Node)
		{
			HarvestState = EGreyfieldHarvestState::Idle;
			break;
		}

		const int32 WantAmount = FMath::Max(1, FMath::CeilToInt(HarvestRate * DeltaTime));
		const int32 Gathered = Node->Harvest(FMath::Min(WantAmount, CarryCapacity - CarriedAmount));
		CarriedAmount += Gathered;

		if (CarriedAmount >= CarryCapacity || Node->IsDepleted())
		{
			DepositTarget = FindNearestDepositPoint();
			if (AGreyfieldBuilding* Deposit = DepositTarget.Get())
			{
				MoveToLocation(Deposit->GetActorLocation());
				HarvestState = EGreyfieldHarvestState::ReturningToDeposit;
			}
			else
			{
				// No deposit point yet - hold the load rather than losing it.
				HarvestState = EGreyfieldHarvestState::Idle;
			}
		}
		break;
	}
	case EGreyfieldHarvestState::ReturningToDeposit:
	{
		AGreyfieldBuilding* Deposit = DepositTarget.Get();
		if (!Deposit)
		{
			HarvestState = EGreyfieldHarvestState::Idle;
			break;
		}
		if (FVector::Dist(GetActorLocation(), Deposit->GetActorLocation()) <= GatherAcceptanceRadius)
		{
			HarvestState = EGreyfieldHarvestState::Depositing;
		}
		break;
	}
	case EGreyfieldHarvestState::Depositing:
	{
		APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
		if (AGreyfieldPlayerState* PS = PC ? Cast<AGreyfieldPlayerState>(PC->PlayerState) : nullptr)
		{
			PS->AddPrimaryResource(CarriedAmount);
		}
		CarriedAmount = 0;

		AGreyfieldResourceNode* Node = TargetNode.Get();
		if (Node && !Node->IsDepleted())
		{
			MoveToLocation(Node->GetActorLocation());
			HarvestState = EGreyfieldHarvestState::MovingToResource;
		}
		else
		{
			HarvestState = EGreyfieldHarvestState::Idle;
		}
		break;
	}
	case EGreyfieldHarvestState::Idle:
	default:
		break;
	}
}
