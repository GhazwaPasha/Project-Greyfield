// Operation Greyfield - player controller.

#include "GreyfieldPlayerController.h"
#include "GreyfieldUnit.h"
#include "GreyfieldHarvesterUnit.h"
#include "GreyfieldResourceNode.h"
#include "GreyfieldBuilding.h"
#include "GreyfieldPlayerState.h"
#include "GreyfieldSelectableInterface.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"

AGreyfieldPlayerController::AGreyfieldPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void AGreyfieldPlayerController::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(FogOfWarTimerHandle, this, &AGreyfieldPlayerController::UpdateFogOfWar, 0.25f, true);

	// Resolved at runtime (not via ConstructorHelpers) so the widget blueprint can be
	// created/edited after this class compiles without needing another recompile - it
	// just needs to exist on disk by the time BeginPlay runs.
	if (UClass* WidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/UI/WBP_GreyfieldHUD.WBP_GreyfieldHUD_C")))
	{
		HUDWidgetInstance = CreateWidget<UUserWidget>(this, WidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
		}
	}
}

void AGreyfieldPlayerController::UpdateFogOfWar()
{
	TArray<AGreyfieldUnit*> PlayerUnits;
	for (TActorIterator<AGreyfieldUnit> It(GetWorld()); It; ++It)
	{
		if (It->Team == EGreyfieldTeam::Player && !It->IsDead())
		{
			PlayerUnits.Add(*It);
		}
	}

	for (TActorIterator<AGreyfieldUnit> It(GetWorld()); It; ++It)
	{
		AGreyfieldUnit* Unit = *It;
		if (Unit->Team != EGreyfieldTeam::Enemy)
		{
			continue;
		}

		bool bVisible = false;
		for (AGreyfieldUnit* PlayerUnit : PlayerUnits)
		{
			const float VisionSq = FMath::Square(PlayerUnit->VisionRadius);
			if (FVector::DistSquared(Unit->GetActorLocation(), PlayerUnit->GetActorLocation()) <= VisionSq)
			{
				bVisible = true;
				break;
			}
		}
		Unit->SetActorHiddenInGame(!bVisible);
	}
}

void AGreyfieldPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AGreyfieldPlayerController::OnLeftMouseDown);
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &AGreyfieldPlayerController::OnLeftMouseUp);
	InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &AGreyfieldPlayerController::OnRightMouseDown);
	InputComponent->BindKey(EKeys::B, IE_Pressed, this, &AGreyfieldPlayerController::OnBuildKeyPressed);
	InputComponent->BindKey(EKeys::Q, IE_Pressed, this, &AGreyfieldPlayerController::OnQueueKeyPressed);
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AGreyfieldPlayerController::OnCancelPlacement);

	InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AGreyfieldPlayerController::OnPressGroup1);
	InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AGreyfieldPlayerController::OnPressGroup2);
	InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AGreyfieldPlayerController::OnPressGroup3);
	InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &AGreyfieldPlayerController::OnPressGroup4);
	InputComponent->BindKey(EKeys::Five, IE_Pressed, this, &AGreyfieldPlayerController::OnPressGroup5);
	InputComponent->BindKey(EKeys::Six, IE_Pressed, this, &AGreyfieldPlayerController::OnPressGroup6);
	InputComponent->BindKey(EKeys::Seven, IE_Pressed, this, &AGreyfieldPlayerController::OnPressGroup7);
	InputComponent->BindKey(EKeys::Eight, IE_Pressed, this, &AGreyfieldPlayerController::OnPressGroup8);
	InputComponent->BindKey(EKeys::Nine, IE_Pressed, this, &AGreyfieldPlayerController::OnPressGroup9);
}

void AGreyfieldPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (bIsDragging)
	{
		float MouseX, MouseY;
		if (GetMousePosition(MouseX, MouseY))
		{
			CurrentDragScreenPos = FVector2D(MouseX, MouseY);
		}
	}

	if (bIsPlacingBuilding)
	{
		UpdatePlacementGhost();
	}
}

bool AGreyfieldPlayerController::GetSelectionDragRect(FVector2D& OutStart, FVector2D& OutCurrent) const
{
	if (!bIsDragging)
	{
		return false;
	}
	if (FVector2D::Distance(DragStartScreenPos, CurrentDragScreenPos) < ClickDragThresholdPx)
	{
		return false;
	}
	OutStart = DragStartScreenPos;
	OutCurrent = CurrentDragScreenPos;
	return true;
}

void AGreyfieldPlayerController::OnLeftMouseDown()
{
	if (bIsPlacingBuilding)
	{
		return;
	}

	bIsDragging = true;
	float MouseX, MouseY;
	if (GetMousePosition(MouseX, MouseY))
	{
		DragStartScreenPos = FVector2D(MouseX, MouseY);
		CurrentDragScreenPos = DragStartScreenPos;
	}
}

void AGreyfieldPlayerController::OnLeftMouseUp()
{
	if (bIsPlacingBuilding)
	{
		ConfirmPlacement();
		return;
	}

	if (!bIsDragging)
	{
		return;
	}
	bIsDragging = false;

	TArray<AActor*> NewSelection;
	const float DragDistance = FVector2D::Distance(DragStartScreenPos, CurrentDragScreenPos);

	if (DragDistance < ClickDragThresholdPx)
	{
		FVector WorldOrigin, WorldDirection;
		if (DeprojectMousePositionToWorld(WorldOrigin, WorldDirection))
		{
			FHitResult Hit;
			FCollisionQueryParams Params(SCENE_QUERY_STAT(GreyfieldSelectClick), true);
			const FVector TraceEnd = WorldOrigin + WorldDirection * 100000.f;
			if (GetWorld()->LineTraceSingleByChannel(Hit, WorldOrigin, TraceEnd, ECC_Visibility, Params))
			{
				if (AActor* HitActor = Hit.GetActor())
				{
					if (HitActor->Implements<UGreyfieldSelectableInterface>())
					{
						NewSelection.Add(HitActor);
					}
				}
			}
		}
	}
	else
	{
		const float MinX = FMath::Min(DragStartScreenPos.X, CurrentDragScreenPos.X);
		const float MinY = FMath::Min(DragStartScreenPos.Y, CurrentDragScreenPos.Y);
		const float MaxX = FMath::Max(DragStartScreenPos.X, CurrentDragScreenPos.X);
		const float MaxY = FMath::Max(DragStartScreenPos.Y, CurrentDragScreenPos.Y);

		for (TActorIterator<AGreyfieldUnit> It(GetWorld()); It; ++It)
		{
			AGreyfieldUnit* Unit = *It;
			FVector2D ScreenPos;
			if (UGameplayStatics::ProjectWorldToScreen(this, Unit->GetActorLocation(), ScreenPos))
			{
				if (ScreenPos.X >= MinX && ScreenPos.X <= MaxX && ScreenPos.Y >= MinY && ScreenPos.Y <= MaxY)
				{
					NewSelection.Add(Unit);
				}
			}
		}
	}

	UpdateSelection(NewSelection);
}

void AGreyfieldPlayerController::OnRightMouseDown()
{
	if (bIsPlacingBuilding)
	{
		OnCancelPlacement();
		return;
	}

	if (SelectedUnits.Num() == 0)
	{
		return;
	}

	FVector WorldOrigin, WorldDirection;
	if (!DeprojectMousePositionToWorld(WorldOrigin, WorldDirection))
	{
		return;
	}

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(GreyfieldMoveOrder), true);
	const FVector TraceEnd = WorldOrigin + WorldDirection * 100000.f;
	if (!GetWorld()->LineTraceSingleByChannel(Hit, WorldOrigin, TraceEnd, ECC_Visibility, Params))
	{
		return;
	}

	if (AGreyfieldResourceNode* Node = Cast<AGreyfieldResourceNode>(Hit.GetActor()))
	{
		bool bAnyHarvester = false;
		for (const TWeakObjectPtr<AActor>& Weak : SelectedUnits)
		{
			if (AGreyfieldHarvesterUnit* Harvester = Cast<AGreyfieldHarvesterUnit>(Weak.Get()))
			{
				Harvester->OrderHarvest(Node);
				bAnyHarvester = true;
			}
		}
		if (bAnyHarvester)
		{
			return;
		}
	}

	if (AGreyfieldUnit* TargetUnit = Cast<AGreyfieldUnit>(Hit.GetActor()))
	{
		bool bAnyAttacker = false;
		for (const TWeakObjectPtr<AActor>& Weak : SelectedUnits)
		{
			AGreyfieldUnit* Attacker = Cast<AGreyfieldUnit>(Weak.Get());
			if (Attacker && Attacker->Team != TargetUnit->Team)
			{
				Attacker->AttackMoveToTarget(TargetUnit);
				bAnyAttacker = true;
			}
		}
		if (bAnyAttacker)
		{
			return;
		}
	}

	FormationMoveOrder(Hit.Location);
}

void AGreyfieldPlayerController::OnBuildKeyPressed()
{
	if (bIsPlacingBuilding)
	{
		return;
	}

	FVector GhostSpawnLocation(0.f);
	GetGroundLocationUnderMouse(GhostSpawnLocation);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	PlacementGhost = GetWorld()->SpawnActor<AGreyfieldBuilding>(AGreyfieldBuilding::StaticClass(), GhostSpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (PlacementGhost)
	{
		PlacementGhost->SetActorEnableCollision(false);
		bIsPlacingBuilding = true;
	}
}

AGreyfieldBuilding* AGreyfieldPlayerController::GetSelectedBuilding() const
{
	if (SelectedUnits.Num() != 1)
	{
		return nullptr;
	}
	return Cast<AGreyfieldBuilding>(SelectedUnits[0].Get());
}

void AGreyfieldPlayerController::OnQueueKeyPressed()
{
	if (AGreyfieldBuilding* Building = GetSelectedBuilding())
	{
		Building->QueueUnit(FName(TEXT("Harvester")));
	}
}

void AGreyfieldPlayerController::OnCancelPlacement()
{
	if (!bIsPlacingBuilding)
	{
		return;
	}
	if (PlacementGhost)
	{
		PlacementGhost->Destroy();
		PlacementGhost = nullptr;
	}
	bIsPlacingBuilding = false;
}

void AGreyfieldPlayerController::UpdatePlacementGhost()
{
	if (!PlacementGhost)
	{
		return;
	}
	FVector GroundLocation;
	if (GetGroundLocationUnderMouse(GroundLocation))
	{
		PlacementGhost->SetActorLocation(GroundLocation);
	}
}

void AGreyfieldPlayerController::ConfirmPlacement()
{
	if (!PlacementGhost)
	{
		bIsPlacingBuilding = false;
		return;
	}

	AGreyfieldPlayerState* PS = Cast<AGreyfieldPlayerState>(PlayerState);
	if (!PS || !PS->SpendPrimaryResource(PlaceholderBuildingCost))
	{
		// Can't afford it - leave placement mode active so the player can reposition
		// or cancel once they notice, rather than silently eating the click.
		return;
	}

	PlacementGhost->SetActorEnableCollision(true);
	PlacementGhost = nullptr;
	bIsPlacingBuilding = false;
}

bool AGreyfieldPlayerController::GetGroundLocationUnderMouse(FVector& OutLocation) const
{
	FVector WorldOrigin, WorldDirection;
	if (!DeprojectMousePositionToWorld(WorldOrigin, WorldDirection))
	{
		return false;
	}

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(GreyfieldMoveOrder), true);
	const FVector TraceEnd = WorldOrigin + WorldDirection * 100000.f;
	if (GetWorld()->LineTraceSingleByChannel(Hit, WorldOrigin, TraceEnd, ECC_Visibility, Params))
	{
		OutLocation = Hit.Location;
		return true;
	}
	return false;
}

void AGreyfieldPlayerController::UpdateSelection(const TArray<AActor*>& NewSelection)
{
	for (const TWeakObjectPtr<AActor>& Weak : SelectedUnits)
	{
		if (AActor* Actor = Weak.Get())
		{
			if (!NewSelection.Contains(Actor) && Actor->Implements<UGreyfieldSelectableInterface>())
			{
				IGreyfieldSelectableInterface::Execute_SetSelected(Actor, false);
			}
		}
	}

	SelectedUnits.Reset();
	for (AActor* Actor : NewSelection)
	{
		if (Actor && Actor->Implements<UGreyfieldSelectableInterface>())
		{
			IGreyfieldSelectableInterface::Execute_SetSelected(Actor, true);
			SelectedUnits.Add(Actor);
		}
	}
}

void AGreyfieldPlayerController::FormationMoveOrder(const FVector& Destination)
{
	const int32 NumUnits = SelectedUnits.Num();
	if (NumUnits == 0)
	{
		return;
	}

	const int32 UnitsPerRow = FMath::Max(1, FMath::CeilToInt(FMath::Sqrt(static_cast<float>(NumUnits))));
	int32 Index = 0;
	for (const TWeakObjectPtr<AActor>& Weak : SelectedUnits)
	{
		AGreyfieldUnit* Unit = Cast<AGreyfieldUnit>(Weak.Get());
		if (!Unit)
		{
			++Index;
			continue;
		}

		const int32 Row = Index / UnitsPerRow;
		const int32 Col = Index % UnitsPerRow;
		const float OffsetX = (Col - (UnitsPerRow - 1) / 2.f) * FormationSpacing;
		const float OffsetY = (Row - (UnitsPerRow - 1) / 2.f) * FormationSpacing;

		Unit->MoveToLocation(Destination + FVector(OffsetX, OffsetY, 0.f));
		++Index;
	}
}

void AGreyfieldPlayerController::OnNumberKeyPressed(int32 Number)
{
	const bool bCtrlHeld = IsInputKeyDown(EKeys::LeftControl) || IsInputKeyDown(EKeys::RightControl);

	if (bCtrlHeld)
	{
		FGreyfieldControlGroupUnits Group;
		Group.Units = SelectedUnits;
		ControlGroups.Add(Number, Group);
		return;
	}

	if (const FGreyfieldControlGroupUnits* Group = ControlGroups.Find(Number))
	{
		TArray<AActor*> NewSelection;
		for (const TWeakObjectPtr<AActor>& Weak : Group->Units)
		{
			if (AActor* Actor = Weak.Get())
			{
				NewSelection.Add(Actor);
			}
		}
		UpdateSelection(NewSelection);
	}
}

void AGreyfieldPlayerController::OnPressGroup1() { OnNumberKeyPressed(1); }
void AGreyfieldPlayerController::OnPressGroup2() { OnNumberKeyPressed(2); }
void AGreyfieldPlayerController::OnPressGroup3() { OnNumberKeyPressed(3); }
void AGreyfieldPlayerController::OnPressGroup4() { OnNumberKeyPressed(4); }
void AGreyfieldPlayerController::OnPressGroup5() { OnNumberKeyPressed(5); }
void AGreyfieldPlayerController::OnPressGroup6() { OnNumberKeyPressed(6); }
void AGreyfieldPlayerController::OnPressGroup7() { OnNumberKeyPressed(7); }
void AGreyfieldPlayerController::OnPressGroup8() { OnNumberKeyPressed(8); }
void AGreyfieldPlayerController::OnPressGroup9() { OnNumberKeyPressed(9); }
