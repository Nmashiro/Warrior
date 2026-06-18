// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HeroGameplayAbility_Cheat.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "WarriorGameplayTags.h"

UHeroGameplayAbility_Cheat::UHeroGameplayAbility_Cheat()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UHeroGameplayAbility_Cheat::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	bHasEnded = false;
	bCheatBuffWasActive = false;

	UWarriorAbilitySystemComponent* WarriorASC = GetWarriorAbilitySystemComponentFromActorInfo();
	if (!WarriorASC || !CheatBuffEffectClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	RemoveCheatBuffEffect(WarriorASC);

	FGameplayEffectContextHandle EffectContextHandle = WarriorASC->MakeEffectContext();
	EffectContextHandle.SetAbility(this);
	EffectContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());
	EffectContextHandle.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());

	const FGameplayEffectSpecHandle EffectSpecHandle = WarriorASC->MakeOutgoingSpec(
		CheatBuffEffectClass,
		GetAbilityLevel(),
		EffectContextHandle
	);

	if (!EffectSpecHandle.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActiveCheatBuffHandle = WarriorASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	if (!ActiveCheatBuffHandle.WasSuccessfullyApplied())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bCheatBuffWasActive = true;

	if (!WarriorASC->HasMatchingGameplayTag(WarriorGameplayTags::Player_Status_Cheater))
	{
		WarriorASC->AddLooseGameplayTag(WarriorGameplayTags::Player_Status_Cheater);
	}

	BP_OnCheatActivated();
}

void UHeroGameplayAbility_Cheat::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	if (bHasEnded)
	{
		return;
	}

	bHasEnded = true;

	if (UWarriorAbilitySystemComponent* WarriorASC = GetWarriorAbilitySystemComponentFromActorInfo())
	{
		RemoveCheatBuffEffect(WarriorASC);
	}

	if (bCheatBuffWasActive)
	{
		BP_OnCheatEnded(bWasCancelled);
	}

	bCheatBuffWasActive = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHeroGameplayAbility_Cheat::RemoveCheatBuffEffect(UWarriorAbilitySystemComponent* WarriorASC)
{
	if (!WarriorASC || !CheatBuffEffectClass)
	{
		return;
	}

	if (ActiveCheatBuffHandle.IsValid())
	{
		WarriorASC->RemoveActiveGameplayEffect(ActiveCheatBuffHandle);
		ActiveCheatBuffHandle.Invalidate();
	}

	WarriorASC->RemoveActiveGameplayEffectBySourceEffect(CheatBuffEffectClass, WarriorASC, -1);
}
