// Operation Greyfield - implemented by anything the player can box-select or click-select.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GreyfieldSelectableInterface.generated.h"

UINTERFACE(BlueprintType)
class PROJECTGREYFIELD_API UGreyfieldSelectableInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECTGREYFIELD_API IGreyfieldSelectableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Greyfield|Selection")
	void SetSelected(bool bSelected);

	UFUNCTION(BlueprintNativeEvent, Category = "Greyfield|Selection")
	bool IsSelected() const;
};
