// Operation Greyfield - tracks a player's resource stockpiles. Primary resource is the
// harvestable economy resource; secondary is the power-style gate from the design doc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GreyfieldPlayerState.generated.h"

UCLASS()
class PROJECTGREYFIELD_API AGreyfieldPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Greyfield|Economy")
	void AddPrimaryResource(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Greyfield|Economy")
	bool SpendPrimaryResource(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Greyfield|Economy")
	int32 GetPrimaryResource() const { return PrimaryResource; }

	UFUNCTION(BlueprintCallable, Category = "Greyfield|Economy")
	void AddSecondaryResource(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Greyfield|Economy")
	int32 GetSecondaryResource() const { return SecondaryResource; }

protected:
	// Starts with a stockpile so the build/queue loop is testable before a harvest loop
	// has had time to run; drop to 0 once the economy needs to be a real constraint.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Greyfield|Economy")
	int32 PrimaryResource = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Greyfield|Economy")
	int32 SecondaryResource = 0;
};
