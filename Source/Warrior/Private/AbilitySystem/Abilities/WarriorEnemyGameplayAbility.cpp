// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WarriorEnemyGameplayAbility.h"
#include "Characters/WarriorEnemyCharacter.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"

AWarriorEnemyCharacter* UWarriorEnemyGameplayAbility::GetEnemyCharacterFromActorInfo()
{
	if (!CachedWarriorEnemyCharacter.IsValid())
	{
		CachedWarriorEnemyCharacter = Cast<AWarriorEnemyCharacter>(CurrentActorInfo->AvatarActor);
	}
	return CachedWarriorEnemyCharacter.IsValid() ? CachedWarriorEnemyCharacter.Get() : nullptr;
}

UEnemyCombatComponent* UWarriorEnemyGameplayAbility::GetEnemyCombatComponentFromActorInfo()
{
	return GetEnemyCharacterFromActorInfo()->GetEnemyCombatComponent();
}

FGameplayEffectSpecHandle UWarriorEnemyGameplayAbility::MakeEnemyDamageEffectSpecHandle(TSubclassOf<UGameplayEffect> EffectClass, const FScalableFloat& InDamageScalableFloat)
{
	check(EffectClass);

	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
	ContextHandle.SetAbility(this);
	ContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());
	ContextHandle.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());

	FGameplayEffectSpecHandle EffectSpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
		EffectClass,
		GetAbilityLevel(),
		ContextHandle
	);

	EffectSpecHandle.Data->SetSetByCallerMagnitude(
		WarriorGameplayTags::Shared_SetByCaller_BaseDamage,
		InDamageScalableFloat.GetValueAtLevel(GetAbilityLevel())
	);
	return EffectSpecHandle;
}

bool UWarriorEnemyGameplayAbility::SelectEnemySummonConfigByCurrentHealth(const TArray<FWarriorEnemySummonHealthConfig>& InSummonConfigs, FWarriorEnemySummonHealthConfig& OutSummonConfig)
{
	AWarriorEnemyCharacter* EnemyCharacter = GetEnemyCharacterFromActorInfo();
	if (!EnemyCharacter)
	{
		return false;
	}

	return UWarriorFunctionLibrary::SelectEnemySummonConfigByActorHealth(EnemyCharacter, InSummonConfigs, OutSummonConfig);
}

bool UWarriorEnemyGameplayAbility::SelectConfiguredEnemySummonConfigByCurrentHealth(FWarriorEnemySummonHealthConfig& OutSummonConfig)
{
	return NativeSelectConfiguredEnemySummonConfigByCurrentHealth(OutSummonConfig);
}

bool UWarriorEnemyGameplayAbility::SelectConfiguredEnemySummonConfigAndIndexByCurrentHealth(FWarriorEnemySummonHealthConfig& OutSummonConfig, int32& OutConfigIndex)
{
	return NativeSelectConfiguredEnemySummonConfigByCurrentHealth(OutSummonConfig, OutConfigIndex);
}

bool UWarriorEnemyGameplayAbility::NativeSelectConfiguredEnemySummonConfigByCurrentHealth(FWarriorEnemySummonHealthConfig& OutSummonConfig)
{
	int32 ConfigIndex = INDEX_NONE;
	return NativeSelectConfiguredEnemySummonConfigByCurrentHealth(OutSummonConfig, ConfigIndex);
}

bool UWarriorEnemyGameplayAbility::NativeSelectConfiguredEnemySummonConfigByCurrentHealth(FWarriorEnemySummonHealthConfig& OutSummonConfig, int32& OutConfigIndex)
{
	return UWarriorFunctionLibrary::NativeSelectEnemySummonConfigByActorHealth(
		GetEnemyCharacterFromActorInfo(),
		EnemySummonHealthConfigs,
		OutSummonConfig,
		OutConfigIndex
	);
}
