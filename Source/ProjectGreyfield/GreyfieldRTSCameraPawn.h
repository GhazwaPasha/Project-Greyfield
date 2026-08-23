// Operation Greyfield - top-down RTS camera pawn: edge-scroll, drag-pan (middle mouse),
// zoom (mouse wheel), and keyboard nudge (WASD/arrows). Raw key/axis bindings are used
// throughout instead of Enhanced Input assets, since input mapping context/action assets
// aren't something the MCP toolset can create.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GreyfieldRTSCameraPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class PROJECTGREYFIELD_API AGreyfieldRTSCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AGreyfieldRTSCameraPawn();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Camera")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Greyfield|Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, Category = "Greyfield|Camera")
	float PanSpeed = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Greyfield|Camera")
	bool bEdgeScrollEnabled = true;

	UPROPERTY(EditAnywhere, Category = "Greyfield|Camera")
	float EdgeScrollMarginPx = 12.f;

	UPROPERTY(EditAnywhere, Category = "Greyfield|Camera")
	float DragPanSpeed = 6.f;

	UPROPERTY(EditAnywhere, Category = "Greyfield|Camera")
	float ZoomStep = 150.f;

	UPROPERTY(EditAnywhere, Category = "Greyfield|Camera")
	float MinZoom = 600.f;

	UPROPERTY(EditAnywhere, Category = "Greyfield|Camera")
	float MaxZoom = 5000.f;

private:
	void OnZoom(float Value);
	void OnDragPanStart();
	void OnDragPanEnd();

	bool bIsDragPanning = false;
	FVector2D LastDragPanMousePos = FVector2D::ZeroVector;
};
