// Operation Greyfield - top-down RTS camera pawn.

#include "GreyfieldRTSCameraPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"

AGreyfieldRTSCameraPawn::AGreyfieldRTSCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// Fixed top-down-ish angle for now; zoom/pitch tuning happens once real levels exist.
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(SceneRoot);
	SpringArm->TargetArmLength = 2000.f;
	SpringArm->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	SpringArm->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
}

void AGreyfieldRTSCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	FVector PanDirection = FVector::ZeroVector;

	// Keyboard nudge - WASD and arrow keys both work.
	if (PC->IsInputKeyDown(EKeys::W) || PC->IsInputKeyDown(EKeys::Up))    PanDirection.X += 1.f;
	if (PC->IsInputKeyDown(EKeys::S) || PC->IsInputKeyDown(EKeys::Down))  PanDirection.X -= 1.f;
	if (PC->IsInputKeyDown(EKeys::D) || PC->IsInputKeyDown(EKeys::Right)) PanDirection.Y += 1.f;
	if (PC->IsInputKeyDown(EKeys::A) || PC->IsInputKeyDown(EKeys::Left))  PanDirection.Y -= 1.f;

	// Edge-scroll - suppressed while drag-panning so the two don't fight.
	if (bEdgeScrollEnabled && !bIsDragPanning)
	{
		float MouseX, MouseY;
		if (PC->GetMousePosition(MouseX, MouseY))
		{
			int32 ViewportSizeX, ViewportSizeY;
			PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
			if (ViewportSizeX > 0 && ViewportSizeY > 0)
			{
				if (MouseX <= EdgeScrollMarginPx)                       PanDirection.Y -= 1.f;
				else if (MouseX >= ViewportSizeX - EdgeScrollMarginPx)  PanDirection.Y += 1.f;
				if (MouseY <= EdgeScrollMarginPx)                       PanDirection.X += 1.f;
				else if (MouseY >= ViewportSizeY - EdgeScrollMarginPx)  PanDirection.X -= 1.f;
			}
		}
	}

	if (!PanDirection.IsNearlyZero())
	{
		PanDirection = PanDirection.GetSafeNormal();
		SetActorLocation(GetActorLocation() + PanDirection * PanSpeed * DeltaTime);
	}

	if (bIsDragPanning)
	{
		float MouseX, MouseY;
		if (PC->GetMousePosition(MouseX, MouseY))
		{
			const FVector2D CurrentMousePos(MouseX, MouseY);
			const FVector2D Delta = CurrentMousePos - LastDragPanMousePos;
			// Screen Y down -> world -X (pan away from where you dragged, like grabbing the map).
			const FVector DragPan = FVector(-Delta.Y, -Delta.X, 0.f) * DragPanSpeed;
			SetActorLocation(GetActorLocation() + DragPan);
			LastDragPanMousePos = CurrentMousePos;
		}
	}
}

void AGreyfieldRTSCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxisKey(EKeys::MouseWheelAxis, this, &AGreyfieldRTSCameraPawn::OnZoom);
	PlayerInputComponent->BindKey(EKeys::MiddleMouseButton, IE_Pressed, this, &AGreyfieldRTSCameraPawn::OnDragPanStart);
	PlayerInputComponent->BindKey(EKeys::MiddleMouseButton, IE_Released, this, &AGreyfieldRTSCameraPawn::OnDragPanEnd);
}

void AGreyfieldRTSCameraPawn::OnZoom(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}
	const float NewLength = SpringArm->TargetArmLength - Value * ZoomStep;
	SpringArm->TargetArmLength = FMath::Clamp(NewLength, MinZoom, MaxZoom);
}

void AGreyfieldRTSCameraPawn::OnDragPanStart()
{
	bIsDragPanning = true;
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		float MouseX, MouseY;
		if (PC->GetMousePosition(MouseX, MouseY))
		{
			LastDragPanMousePos = FVector2D(MouseX, MouseY);
		}
	}
}

void AGreyfieldRTSCameraPawn::OnDragPanEnd()
{
	bIsDragPanning = false;
}
