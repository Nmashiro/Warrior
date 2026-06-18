// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WarriorHeroGameplayAbility.h"
#include "GameplayEffectTypes.h"
#include "HeroGameplayAbility_Cheat.generated.h"

class UGameplayEffect;
class UWarriorAbilitySystemComponent;

/** Toggleable cheat ability. The cheater tag stays forever, while the buff is removed when the ability ends. */
UCLASS()
class WARRIOR_API UHeroGameplayAbility_Cheat : public UWarriorHeroGameplayAbility
{
	GENERATED_BODY()

public:
	UHeroGameplayAbility_Cheat();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat")
	TSubclassOf<UGameplayEffect> CheatBuffEffectClass;

	UPROPERTY(BlueprintReadOnly, Category = "Cheat")
	FActiveGameplayEffectHandle ActiveCheatBuffHandle;

private:
	void RemoveCheatBuffEffect(UWarriorAbilitySystemComponent* WarriorASC);

	bool bHasEnded = true;
	bool bCheatBuffWasActive = false;

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Cheat")
	void BP_OnCheatActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cheat")
	void BP_OnCheatEnded(bool bWasCancelled);
};
