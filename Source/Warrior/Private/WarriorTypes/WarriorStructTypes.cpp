// Fill out your copyright notice in the Description page of Project Settings.


#include "WarriorTypes/WarriorStructTypes.h"
#include "AbilitySystem/Abilities/WarriorHeroGameplayAbility.h"

bool FWarriorEnemySummonInfo::IsValid() const
{
	return !SoftEnemyClassToSpawn.IsNull() && NumToSpawn > 0;
}

bool FWarriorEnemySummonHealthConfig::IsHealthPercentInRange(float InHealthPercent) const
{
	const float ClampedHealthPercent = FMath::Clamp(InHealthPercent, 0.f, 1.f);
	const float ClampedMin = FMath::Clamp(MinHealthPercent, 0.f, 1.f);
	const float ClampedMax = FMath::Clamp(MaxHealthPercent, 0.f, 1.f);

	return ClampedHealthPercent >= ClampedMin && ClampedHealthPercent <= ClampedMax;
}

bool FWarriorEnemySummonHealthConfig::IsSingleHealthThreshold() const
{
	return FMath::IsNearlyEqual(
		FMath::Clamp(MinHealthPercent, 0.f, 1.f),
		FMath::Clamp(MaxHealthPercent, 0.f, 1.f)
	);
}

bool FWarriorEnemySummonHealthConfig::HasValidEnemiesToSpawn() const
{
	for (const FWarriorEnemySummonInfo& EnemySummonInfo : EnemiesToSpawn)
	{
		if (EnemySummonInfo.IsValid())
		{
			return true;
		}
	}

	return false;
}

bool FWarriorHeroAbilitySet::IsValid() const
{
	return InputTag.IsValid() && AbilityToGrant;
}
