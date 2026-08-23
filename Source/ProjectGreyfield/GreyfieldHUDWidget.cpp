// Operation Greyfield - HUD panel update logic.

#include "GreyfieldHUDWidget.h"
#include "GreyfieldPlayerState.h"
#include "GreyfieldPlayerController.h"
#include "GreyfieldGameMode.h"
#include "GreyfieldBuilding.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"

void UGreyfieldHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	AGreyfieldPlayerController* PC = GetOwningPlayer<AGreyfieldPlayerController>();
	if (!PC)
	{
		return;
	}

	if (const AGreyfieldPlayerState* PS = Cast<AGreyfieldPlayerState>(PC->PlayerState))
	{
		if (ResourceText)
		{
			ResourceText->SetText(FText::FromString(FString::Printf(TEXT("Resource: %d"), PS->GetPrimaryResource())));
		}
		if (PowerText)
		{
			PowerText->SetText(FText::FromString(FString::Printf(TEXT("Power: %d"), PS->GetSecondaryResource())));
		}
	}

	if (SelectionText)
	{
		SelectionText->SetText(FText::FromString(FString::Printf(TEXT("Selected: %d   [B] Place building"), PC->GetSelectedUnitCount())));
	}

	if (CommandCardText)
	{
		if (AGreyfieldBuilding* Building = PC->GetSelectedBuilding())
		{
			FString Text = Building->IsUnderConstruction()
				? FString::Printf(TEXT("Under construction: %.0f%%"), Building->GetConstructionFraction() * 100.f)
				: TEXT("[Q] Queue Harvester (50)");

			const TArray<FString> Queue = Building->GetQueueSummary();
			if (Queue.Num() > 0)
			{
				Text += TEXT("\nQueue: ") + FString::Join(Queue, TEXT(", "));
			}
			CommandCardText->SetText(FText::FromString(Text));
			CommandCardText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			CommandCardText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (GameOverText)
	{
		const AGreyfieldGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AGreyfieldGameMode>() : nullptr;
		GameOverText->SetText(FText::FromString(TEXT("DEFEAT")));
		GameOverText->SetVisibility((GM && GM->IsMatchOver()) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}
