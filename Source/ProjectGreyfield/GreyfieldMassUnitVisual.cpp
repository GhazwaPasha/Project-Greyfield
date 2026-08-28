// Operation Greyfield - Phase 2 (Mass Entity player integration, 2026-08-29).

#include "GreyfieldMassUnitVisual.h"
#include "GreyfieldMassFragments.h"
#include "GreyfieldMassSubsystem.h"
#include "MassActorSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassEntityManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogGreyfieldMassUnitVisual, Log, All);

AGreyfieldMassUnitVisual::AGreyfieldMassUnitVisual()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	SetRootComponent(BodyMesh);
	BodyMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 1.9f));
	// Only needs to be a click-select/move-order trace target (ECC_Visibility, matching
	// GreyfieldPlayerController's line traces) - not a physical obstacle. Mass's own avoidance
	// system handles unit-to-unit separation, not Unreal physics collision.
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BodyMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	BodyMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	if (CylinderMeshFinder.Succeeded())
	{
		BodyMesh->SetStaticMesh(CylinderMeshFinder.Object);
	}

	SelectionRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectionRing"));
	SelectionRing->SetupAttachment(BodyMesh);
	SelectionRing->SetRelativeLocation(FVector(0.f, 0.f, -0.5f));
	SelectionRing->SetRelativeScale3D(FVector(1.5f, 1.5f, 0.02f));
	SelectionRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SelectionRing->SetVisibility(false);
	if (CylinderMeshFinder.Succeeded())
	{
		SelectionRing->SetStaticMesh(CylinderMeshFinder.Object);
	}
}

void AGreyfieldMassUnitVisual::BeginPlay()
{
	Super::BeginPlay();

	// Dev-visibility only (Phase 2 PIE verification, 2026-08-29): confirms Mass's representation
	// system actually spawned this actor and registered it with UMassActorSubsystem, so
	// GetEntityHandle() resolves - the bridge GreyfieldPlayerController relies on for click-select
	// -> squad command. Checked on a short delay rather than immediately in BeginPlay since actor
	// registration with UMassActorSubsystem happens in the representation system's post-spawn
	// callback, which may run slightly after BeginPlay rather than strictly before it.
	FTimerHandle Unused;
	GetWorldTimerManager().SetTimer(Unused, [this]()
	{
		const FMassEntityHandle Handle = GetEntityHandle();
		UE_LOG(LogGreyfieldMassUnitVisual, Log, TEXT("GreyfieldMassUnitVisual %s at %s: entity handle %s (valid=%d)"),
			*GetName(), *GetActorLocation().ToString(), *Handle.DebugGetDescription(), Handle.IsValid());
	}, 0.1f, false);
}

void AGreyfieldMassUnitVisual::SetSelected_Implementation(bool bSelected)
{
	bIsSelected = bSelected;
	if (SelectionRing)
	{
		SelectionRing->SetVisibility(bSelected);
	}
}

bool AGreyfieldMassUnitVisual::IsSelected_Implementation() const
{
	return bIsSelected;
}

FMassEntityHandle AGreyfieldMassUnitVisual::GetEntityHandle() const
{
	UWorld* World = GetWorld();
	UMassActorSubsystem* ActorSubsystem = World ? World->GetSubsystem<UMassActorSubsystem>() : nullptr;
	if (!ActorSubsystem)
	{
		return FMassEntityHandle();
	}
	return ActorSubsystem->GetEntityHandleFromActor(this);
}

EGreyfieldTeam AGreyfieldMassUnitVisual::GetGreyfieldTeam_Implementation() const
{
	const UWorld* World = GetWorld();
	const UMassEntitySubsystem* MassEntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	const FMassEntityHandle Handle = GetEntityHandle();
	if (!MassEntitySubsystem || !Handle.IsValid())
	{
		return EGreyfieldTeam::Player;
	}
	const FGreyfieldMassAgentFragment* AgentFragment = MassEntitySubsystem->GetEntityManager().GetFragmentDataPtr<FGreyfieldMassAgentFragment>(Handle);
	return AgentFragment ? AgentFragment->Team : EGreyfieldTeam::Player;
}

void AGreyfieldMassUnitVisual::IssueMoveOrder_Implementation(const FVector& Destination)
{
	// Solo squad-of-one fallback - see header/interface comments. Real multi-unit group orders
	// are issued directly by GreyfieldPlayerController via UGreyfieldMassSubsystem instead of
	// going through this per-actor path, so the group gets one real squad formation rather than
	// each unit becoming its own independent squad-of-one.
	UGreyfieldMassSubsystem* MassSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UGreyfieldMassSubsystem>() : nullptr;
	const FMassEntityHandle Handle = GetEntityHandle();
	if (!MassSubsystem || !Handle.IsValid())
	{
		return;
	}
	const TArray<FMassEntityHandle> Solo = { Handle };
	MassSubsystem->FormSquad(Solo);
	MassSubsystem->IssueSquadMoveOrder(Solo, Destination);
}

void AGreyfieldMassUnitVisual::IssueAttackOrder_Implementation(AActor* Target)
{
	// No dedicated Mass "attack-move to target" order yet (known gap) - GreyfieldMassCombatProcessor
	// already auto-engages any opposing-team entity that comes within range via its own spatial
	// hash grid scan, so moving the squad to the target's current location is enough to trigger a
	// real fight once in range, matching how the AI director's attack-move already behaves.
	if (Target)
	{
		IssueMoveOrder_Implementation(Target->GetActorLocation());
	}
}
