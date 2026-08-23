// Operation Greyfield - marquee selection box HUD overlay.

#include "GreyfieldHUD.h"
#include "GreyfieldPlayerController.h"
#include "Engine/Canvas.h"

void AGreyfieldHUD::DrawHUD()
{
	Super::DrawHUD();

	// Resource/selection readout now lives in the real WBP_GreyfieldHUD widget
	// (UGreyfieldHUDWidget) - this HUD class is left owning just the marquee box.
	const AGreyfieldPlayerController* PC = Cast<AGreyfieldPlayerController>(GetOwningPlayerController());
	if (!PC)
	{
		return;
	}

	FVector2D Start, Current;
	if (!PC->GetSelectionDragRect(Start, Current))
	{
		return;
	}

	const float MinX = FMath::Min(Start.X, Current.X);
	const float MinY = FMath::Min(Start.Y, Current.Y);
	const float Width = FMath::Abs(Current.X - Start.X);
	const float Height = FMath::Abs(Current.Y - Start.Y);

	DrawRect(FLinearColor(0.3f, 0.9f, 0.4f, 0.12f), MinX, MinY, Width, Height);
	DrawLine(MinX, MinY, MinX + Width, MinY, FLinearColor::Green, 2.f);
	DrawLine(MinX, MinY + Height, MinX + Width, MinY + Height, FLinearColor::Green, 2.f);
	DrawLine(MinX, MinY, MinX, MinY + Height, FLinearColor::Green, 2.f);
	DrawLine(MinX + Width, MinY, MinX + Width, MinY + Height, FLinearColor::Green, 2.f);
}
