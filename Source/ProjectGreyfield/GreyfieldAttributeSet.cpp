// Operation Greyfield - GAS AttributeSet.

#include "GreyfieldAttributeSet.h"

UGreyfieldAttributeSet::UGreyfieldAttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	// Non-zero placeholder default so every unit can fight out of the box during Phase 4
	// testing; per-unit-type values come from DT_Units once spawn-time attribute init exists.
	InitAttackDamage(15.f);
	InitArmor(0.f);
}

void UGreyfieldAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(1.f, NewValue);
	}
}
