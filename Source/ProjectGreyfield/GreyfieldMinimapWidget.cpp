// Operation Greyfield - minimap paint logic.

#include "GreyfieldMinimapWidget.h"
#include "GreyfieldUnit.h"
#include "GreyfieldBuilding.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

int32 UGreyfieldMinimapWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	LayerId = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
	const FVector2D LocalSize = AllottedGeometry.GetLocalSize();

	// Background panel so the dots read against any 3D content behind it.
	FSlateDrawElement::MakeBox(
		OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(),
		WhiteBrush, ESlateDrawEffect::None, FLinearColor(0.02f, 0.05f, 0.02f, 0.85f));
	++LayerId;

	UWorld* World = GetWorld();
	if (!World)
	{
		return LayerId;
	}

	auto WorldToLocal = [this, &LocalSize](const FVector& WorldLocation) -> FVector2D
	{
		const float NormX = FMath::Clamp((WorldLocation.X + WorldHalfExtent) / (WorldHalfExtent * 2.f), 0.f, 1.f);
		const float NormY = FMath::Clamp((WorldLocation.Y + WorldHalfExtent) / (WorldHalfExtent * 2.f), 0.f, 1.f);
		// Screen-up matches world +X (the RTS camera's fixed yaw), so invert X into local Y.
		return FVector2D(NormY * LocalSize.X, (1.f - NormX) * LocalSize.Y);
	};

	for (TActorIterator<AGreyfieldBuilding> It(World); It; ++It)
	{
		const FVector2D DotPos = WorldToLocal(It->GetActorLocation());
		FSlateDrawElement::MakeBox(
			OutDrawElements, LayerId,
			AllottedGeometry.ToPaintGeometry(FVector2D(DotSize, DotSize), FSlateLayoutTransform(DotPos - FVector2D(DotSize * 0.5f))),
			WhiteBrush, ESlateDrawEffect::None, FLinearColor(0.6f, 0.6f, 0.9f, 1.f));
	}

	for (TActorIterator<AGreyfieldUnit> It(World); It; ++It)
	{
		AGreyfieldUnit* Unit = *It;
		if (!Unit || Unit->IsHidden() || Unit->IsDead())
		{
			continue;
		}

		const FVector2D DotPos = WorldToLocal(Unit->GetActorLocation());
		const FLinearColor DotColor = (Unit->Team == EGreyfieldTeam::Player) ? FLinearColor(0.3f, 0.9f, 0.4f, 1.f) : FLinearColor(0.9f, 0.25f, 0.25f, 1.f);

		FSlateDrawElement::MakeBox(
			OutDrawElements, LayerId,
			AllottedGeometry.ToPaintGeometry(FVector2D(DotSize, DotSize), FSlateLayoutTransform(DotPos - FVector2D(DotSize * 0.5f))),
			WhiteBrush, ESlateDrawEffect::None, DotColor);
	}
	++LayerId;

	return LayerId;
}
