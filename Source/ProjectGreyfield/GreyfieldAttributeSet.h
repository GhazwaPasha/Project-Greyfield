// Operation Greyfield - GAS AttributeSet shared by every unit and building capable of
// combat. Damage is applied directly (SetHealth via the ASC) rather than through a
// GameplayEffect asset for now - correct GAS scaffolding, simplified application path.
// Swap in real GameplayEffects once the combat loop itself is proven.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GreyfieldAttributeSet.generated.h"

#define GREYFIELD_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class PROJECTGREYFIELD_API UGreyfieldAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UGreyfieldAttributeSet();

	UPROPERTY(BlueprintReadOnly, Category = "Greyfield|Attributes")
	FGameplayAttributeData Health;
	GREYFIELD_ATTRIBUTE_ACCESSORS(UGreyfieldAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Greyfield|Attributes")
	FGameplayAttributeData MaxHealth;
	GREYFIELD_ATTRIBUTE_ACCESSORS(UGreyfieldAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category = "Greyfield|Attributes")
	FGameplayAttributeData AttackDamage;
	GREYFIELD_ATTRIBUTE_ACCESSORS(UGreyfieldAttributeSet, AttackDamage)

	UPROPERTY(BlueprintReadOnly, Category = "Greyfield|Attributes")
	FGameplayAttributeData Armor;
	GREYFIELD_ATTRIBUTE_ACCESSORS(UGreyfieldAttributeSet, Armor)

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
};
