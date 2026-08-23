// Operation Greyfield - minimap. Drawn directly via NativePaint (colored dots for units)
// rather than a render-target scene capture - far more reliable to get right without
// visual iteration, and it's placeholder-appropriate until the first art pass anyway.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GreyfieldMinimapWidget.generated.h"

UCLASS()
class PROJECTGREYFIELD_API UGreyfieldMinimapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

protected:
	// Half-extent of the playable area in world units - matches the 4000x4000 test floor.
	UPROPERTY(EditAnywhere, Category = "Greyfield|Minimap")
	float WorldHalfExtent = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Greyfield|Minimap")
	float DotSize = 6.f;
};
