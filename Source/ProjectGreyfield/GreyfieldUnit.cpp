// Operation Greyfield - placeholder unit.

#include "GreyfieldUnit.h"
#include "GreyfieldUnitAIController.h"
#include "GreyfieldAttributeSet.h"
#include "GreyfieldBuilding.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "Engine/World.h"

AGreyfieldUnit::AGreyfieldUnit()
{
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AGreyfieldUnitAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(GetCapsuleComponent());
	BodyMesh->SetRelativeLocation(FVector(0.f, 0.f, -96.f));
	BodyMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 1.9f));
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (CylinderMeshFinder.Succeeded())
	{
		BodyMesh->SetStaticMesh(CylinderMeshFinder.Object);
	}

	SelectionRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectionRing"));
	SelectionRing->SetupAttachment(GetCapsuleComponent());
	SelectionRing->SetRelativeLocation(FVector(0.f, 0.f, -95.f));
	SelectionRing->SetRelativeScale3D(FVector(0.9f, 0.9f, 0.02f));
	SelectionRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SelectionRing->SetVisibility(false);
	if (CylinderMeshFinder.Succeeded())
	{
		SelectionRing->SetStaticMesh(CylinderMeshFinder.Object);
	}

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UGreyfieldAttributeSet>(TEXT("AttributeSet"));
}

void AGreyfieldUnit::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

UAbilitySystemComponent* AGreyfieldUnit::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGreyfieldUnit::SetSelected_Implementation(bool bSelected)
{
	bIsSelected = bSelected;
	if (SelectionRing)
	{
		SelectionRing->SetVisibility(bSelected);
	}
}

bool AGreyfieldUnit::IsSelected_Implementation() const
{
	return bIsSelected;
}

EGreyfieldTeam AGreyfieldUnit::GetGreyfieldTeam_Implementation() const
{
	return Team;
}

void AGreyfieldUnit::IssueMoveOrder_Implementation(const FVector& Destination)
{
	MoveToLocation(Destination);
}

void AGreyfieldUnit::IssueAttackOrder_Implementation(AActor* Target)
{
	AttackMoveToTarget(Cast<AGreyfieldUnit>(Target));
}

void AGreyfieldUnit::MoveToLocation(const FVector& Destination)
{
	AttackTarget = nullptr;
	AttackTargetBuilding = nullptr;
	bAggressive = false;
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		AICon->MoveToLocation(Destination, /*AcceptanceRadius=*/50.f);
	}
}

void AGreyfieldUnit::AttackMoveToTarget(AGreyfieldUnit* Target)
{
	if (!Target || Target->IsDead())
	{
		return;
	}
	AttackTarget = Target;
	AttackTargetBuilding = nullptr;
	bAggressive = false;
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		AICon->MoveToActor(Target, /*AcceptanceRadius=*/AttackRange * 0.8f);
	}
}

void AGreyfieldUnit::AttackMoveToLocation(const FVector& Destination)
{
	AttackTarget = nullptr;
	AttackTargetBuilding = nullptr;
	bAggressive = true;
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		AICon->MoveToLocation(Destination, /*AcceptanceRadius=*/50.f);
	}
}

void AGreyfieldUnit::ScanForNearbyEnemy()
{
	AGreyfieldUnit* Nearest = nullptr;
	float BestDistSq = FMath::Square(AttackRange * 1.5f);

	for (TActorIterator<AGreyfieldUnit> It(GetWorld()); It; ++It)
	{
		AGreyfieldUnit* Other = *It;
		if (Other == this || Other->Team == Team || Other->IsDead())
		{
			continue;
		}
		const float DistSq = FVector::DistSquared(GetActorLocation(), Other->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Nearest = Other;
		}
	}

	if (Nearest)
	{
		AttackTarget = Nearest;
		if (AAIController* AICon = Cast<AAIController>(GetController()))
		{
			AICon->MoveToActor(Nearest, /*AcceptanceRadius=*/AttackRange * 0.8f);
		}
		return;
	}

	// No enemy unit in range - buildings have no Team of their own (only the player has
	// one right now), so any building is a valid target while attack-moving.
	AGreyfieldBuilding* NearestBuilding = nullptr;
	float BestBuildingDistSq = FMath::Square(AttackRange * 1.5f);
	for (TActorIterator<AGreyfieldBuilding> It(GetWorld()); It; ++It)
	{
		if (It->IsDestroyed())
		{
			continue;
		}
		const float DistSq = FVector::DistSquared(GetActorLocation(), It->GetActorLocation());
		if (DistSq < BestBuildingDistSq)
		{
			BestBuildingDistSq = DistSq;
			NearestBuilding = *It;
		}
	}

	if (NearestBuilding)
	{
		AttackTargetBuilding = NearestBuilding;
		if (AAIController* AICon = Cast<AAIController>(GetController()))
		{
			AICon->MoveToActor(NearestBuilding, /*AcceptanceRadius=*/AttackRange * 0.8f);
		}
	}
}

void AGreyfieldUnit::ApplyDamage(float DamageAmount, AActor* DamageInstigator)
{
	if (!AttributeSet || DamageAmount <= 0.f)
	{
		return;
	}
	const float EffectiveDamage = FMath::Max(1.f, DamageAmount - AttributeSet->GetArmor());
	AttributeSet->SetHealth(FMath::Max(0.f, AttributeSet->GetHealth() - EffectiveDamage));

	if (IsDead())
	{
		if (AAIController* AICon = Cast<AAIController>(GetController()))
		{
			AICon->StopMovement();
		}
		SetActorEnableCollision(false);
		SetActorTickEnabled(false);
		SetLifeSpan(2.f);
	}
}

bool AGreyfieldUnit::IsDead() const
{
	return AttributeSet && AttributeSet->GetHealth() <= 0.f;
}

void AGreyfieldUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeSinceLastAttack += DeltaTime;

	AGreyfieldUnit* Target = AttackTarget.Get();
	AGreyfieldBuilding* TargetBuilding = AttackTargetBuilding.Get();
	if ((!Target || Target->IsDead()) && (!TargetBuilding || TargetBuilding->IsDestroyed()) && bAggressive)
	{
		ScanForNearbyEnemy();
		Target = AttackTarget.Get();
		TargetBuilding = AttackTargetBuilding.Get();
	}

	const float MyDamage = AttributeSet ? AttributeSet->GetAttackDamage() : 0.f;

	if (Target && !Target->IsDead())
	{
		if (FVector::Dist(GetActorLocation(), Target->GetActorLocation()) > AttackRange)
		{
			return;
		}
		if (TimeSinceLastAttack < AttackInterval)
		{
			return;
		}
		TimeSinceLastAttack = 0.f;
		if (MyDamage > 0.f)
		{
			Target->ApplyDamage(MyDamage, this);
		}
		return;
	}

	if (TargetBuilding && !TargetBuilding->IsDestroyed())
	{
		if (FVector::Dist(GetActorLocation(), TargetBuilding->GetActorLocation()) > AttackRange)
		{
			return;
		}
		if (TimeSinceLastAttack < AttackInterval)
		{
			return;
		}
		TimeSinceLastAttack = 0.f;
		if (MyDamage > 0.f)
		{
			TargetBuilding->ApplyDamage(MyDamage);
		}
	}
}
