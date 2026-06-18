// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WarriorGameplayAbility.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "WarriorEnemyGameplayAbility.generated.h"



class AWarriorEnemyCharacter;
class UEnemyCombatComponent;
/**
 * 
 */
UCLASS()
class WARRIOR_API UWarriorEnemyGameplayAbility : public UWarriorGameplayAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	AWarriorEnemyCharacter* GetEnemyCharacterFromActorInfo();

	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	UEnemyCombatComponent* GetEnemyCombatComponentFromActorInfo();

	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	FGameplayEffectSpecHandle MakeEnemyDamageEffectSpecHandle(TSubclassOf<UGameplayEffect> EffectClass, const FScalableFloat& InDamageScalableFloat);

	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	bool SelectEnemySummonConfigByCurrentHealth(const TArray<FWarriorEnemySummonHealthConfig>& InSummonConfigs, FWarriorEnemySummonHealthConfig& OutSummonConfig);

	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	bool SelectConfiguredEnemySummonConfigByCurrentHealth(FWarriorEnemySummonHealthConfig& OutSummonConfig);

	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	bool SelectConfiguredEnemySummonConfigAndIndexByCurrentHealth(FWarriorEnemySummonHealthConfig& OutSummonConfig, int32& OutConfigIndex);

	bool NativeSelectConfiguredEnemySummonConfigByCurrentHealth(FWarriorEnemySummonHealthConfig& OutSummonConfig);
	bool NativeSelectConfiguredEnemySummonConfigByCurrentHealth(FWarriorEnemySummonHealthConfig& OutSummonConfig, int32& OutConfigIndex);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WarriorAbility|Summon", meta = (TitleProperty = "ConfigName"))
	TArray<FWarriorEnemySummonHealthConfig> EnemySummonHealthConfigs;

private:
	TWeakObjectPtr<AWarriorEnemyCharacter> CachedWarriorEnemyCharacter;
};
