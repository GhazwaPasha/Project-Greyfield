// Operation Greyfield - the real HUD panel, replacing Phase 3's debug text. Update logic
// lives here in C++ (NativeTick) rather than Blueprint graph wiring, so the widget's visual
// tree can be built through the MCP UMG toolset without needing to author any graph logic
// blind. Widget names below (ResourceText etc.) must match exactly what the WBP_GreyfieldHUD
// asset's tree uses for BindWidget to succeed.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GreyfieldHUDWidget.generated.h"

class UTextBlock;

UCLASS()
class PROJECTGREYFIELD_API UGreyfieldHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ResourceText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PowerText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SelectionText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GameOverText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CommandCardText;
};
