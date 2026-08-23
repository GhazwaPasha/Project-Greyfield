// Operation Greyfield - player controller. Owns selection state, box-select and
// click-select, move orders with simple grid-formation offsets, and control groups.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "GreyfieldPlayerController.generated.h"

class AActor;
class UUserWidget;

// Wraps a control group's unit list so it can live as a TMap value (UHT is happier with a
// struct wrapper here than a raw nested TArray-in-TMap).
USTRUCT()
struct FGreyfieldControlGroupUnits
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> Units;
};

UCLASS()
class PROJECTGREYFIELD_API AGreyfieldPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGreyfieldPlayerController();

	// Used by AGreyfieldHUD to draw the marquee box while dragging. Returns false
	// (no rect to draw) if not currently dragging past the click/drag threshold.
	UFUNCTION(BlueprintCallable, Category = "Greyfield|Selection")
	bool GetSelectionDragRect(FVector2D& OutStart, FVector2D& OutCurrent) const;

	UFUNCTION(BlueprintPure, Category = "Greyfield|Selection")
	int32 GetSelectedUnitCount() const { return SelectedUnits.Num(); }

	// Returns the selected building when exactly one is selected, else null. Used by the
	// HUD command card and the queue hotkey.
	UFUNCTION(BlueprintPure, Category = "Greyfield|Selection")
	class AGreyfieldBuilding* GetSelectedBuilding() const;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

private:
	void OnLeftMouseDown();
	void OnLeftMouseUp();
	void OnRightMouseDown();

	// Building placement - press B to start, left-click confirms, right-click/Escape cancels.
	void OnBuildKeyPressed();

	// Queues a Harvester at the currently selected building, if any. Placeholder single-hotkey
	// production until the command card has real per-building-type queue buttons.
	void OnQueueKeyPressed();
	void OnCancelPlacement();
	void ConfirmPlacement();
	void UpdatePlacementGhost();

	void OnNumberKeyPressed(int32 Number);
	void OnPressGroup1(); void OnPressGroup2(); void OnPressGroup3();
	void OnPressGroup4(); void OnPressGroup5(); void OnPressGroup6();
	void OnPressGroup7(); void OnPressGroup8(); void OnPressGroup9();

	void UpdateSelection(const TArray<AActor*>& NewSelection);
	void FormationMoveOrder(const FVector& Destination);
	bool GetGroundLocationUnderMouse(FVector& OutLocation) const;
	void UpdateFogOfWar();

	FTimerHandle FogOfWarTimerHandle;

	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidgetInstance;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> SelectedUnits;

	UPROPERTY()
	TMap<int32, FGreyfieldControlGroupUnits> ControlGroups;

	UPROPERTY(EditAnywhere, Category = "Greyfield|Selection")
	float ClickDragThresholdPx = 8.f;

	UPROPERTY(EditAnywhere, Category = "Greyfield|Movement")
	float FormationSpacing = 150.f;

	// Flat cost for the placeholder test building - real per-building costs come from
	// DT_Buildings once actual building types exist (Phase 4+).
	UPROPERTY(EditAnywhere, Category = "Greyfield|Building")
	int32 PlaceholderBuildingCost = 100;

	bool bIsPlacingBuilding = false;

	UPROPERTY()
	TObjectPtr<AActor> PlacementGhost;

	bool bIsDragging = false;
	FVector2D DragStartScreenPos = FVector2D::ZeroVector;
	FVector2D CurrentDragScreenPos = FVector2D::ZeroVector;
};
