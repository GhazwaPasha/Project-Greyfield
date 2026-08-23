// Operation Greyfield - draws the marquee selection box during a drag-select.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GreyfieldHUD.generated.h"

UCLASS()
class PROJECTGREYFIELD_API AGreyfieldHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
};
