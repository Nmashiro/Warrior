// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/StartUpData/DataAsset_StartUpDataBase.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/WarriorGameplayAbility.h"

void UDataAsset_StartUpDataBase::GiveToAbilitySystemComponent(UWarriorAbilitySystemComponent* InASCToGive, int32 ApplyLevel)
{
	check(InASCToGive);
	GrantAbilities(ActionOnGivenAbilities, InASCToGive, ApplyLevel);
	GrantAbilities(ReactiveAbilities, InASCToGive, ApplyLevel);

	if (!StartUpGameplayEffects.IsEmpty())
	{
		for (TSubclassOf<UGameplayEffect> EffectClass : StartUpGameplayEffects)
		{
			if (!EffectClass) continue;

			UGameplayEffect* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();
			InASCToGive->ApplyGameplayEffectToSelf(
				EffectCDO,
				ApplyLevel,
				InASCToGive->MakeEffectContext()
			);
		}
	}
}

void UDataAsset_StartUpDataBase::GrantAbilities(const TArray<TSubclassOf<UWarriorGameplayAbility>>& InAbilitiesToGive, UWarriorAbilitySystemComponent* InASCToGive, int32 ApplyLevel)
{
	if (InAbilitiesToGive.IsEmpty())
	{
		return;
	}
	for (const TSubclassOf<UWarriorGameplayAbility>& AbilityClass : InAbilitiesToGive)
	{
		if (!AbilityClass) continue;

		FGameplayAbilitySpec AbilitySpec(AbilityClass);
		AbilitySpec.SourceObject = InASCToGive->GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;

		InASCToGive->GiveAbility(AbilitySpec);
		// 授予能力并获取句柄
		//FGameplayAbilitySpecHandle AbilityHandle = InASCToGive->GiveAbility(AbilitySpec);

		//// 读取能力类的默认实例（CDO）里的策略
		//if (AbilityHandle.IsValid())
		//{
		//	UWarriorGameplayAbility* AbilityCDO = AbilityClass->GetDefaultObject<UWarriorGameplayAbility>();
		//	if (AbilityCDO && AbilityCDO->GetActivationPolicy() == EWarriorAbilityActivationPolicy::OnGiven)
		//	{
		//		// 仅在服务端尝试自动激活
		//		AActor* Avatar = InASCToGive->GetAvatarActor();
		//		if (Avatar && Avatar->HasAuthority())
		//		{
		//			InASCToGive->TryActivateAbility(AbilityHandle);
		//		}
		//	}
		//	// 如果是 OnTriggered，就只授予，不自动激活（留给蓝图/输入/事件触发）
		//}
	}
}
