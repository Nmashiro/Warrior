// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WarriorEnemyGameplayAbility.h"
#include "EnemyGameplayAbility_SummonEnemies.generated.h"

class UAnimMontage;
class UAbilityTask_WaitSpawnEnemies;
class UAbilityTask_PlayMontageAndWait;
class UBlackboardComponent;

/**
 * Native summon ability for enemy bosses.
 * Blueprint should only provide configuration data.
 */
UCLASS()
class WARRIOR_API UEnemyGameplayAbility_SummonEnemies : public UWarriorEnemyGameplayAbility
{
	GENERATED_BODY()

public:
	UEnemyGameplayAbility_SummonEnemies();

	virtual void PostLoad() override;
	virtual void PostInitProperties() override;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|Summon")
	TObjectPtr<UAnimMontage> MontageToPlay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|Summon")
	FGameplayTag EventTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|Summon", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float RandomSpawnRadius = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|Summon")
	FName HasSpawnedEnemiesKeyName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|Summon")
	FName SummonPhaseKeyName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|Summon")
	bool bWriteSummonPhaseToBlackboard = true;

private:
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitSpawnEnemies> SpawnEnemiesTask;

	int32 CurrentSummonPhase = INDEX_NONE;
	bool bIsEndingAbility = false;

	UFUNCTION()
	void HandleSpawnFinished(const TArray<AWarriorEnemyCharacter*>& SpawnedEnemies);

	UFUNCTION()
	void HandleSpawnFailed(const TArray<AWarriorEnemyCharacter*>& SpawnedEnemies);

	UFUNCTION()
	void HandleMontageEnded();

	UFUNCTION()
	void HandleMontageInterruptedOrCancelled();

	void MarkSummonFinished(bool bSuccess, int32 SpawnedEnemyCount);
	void EndSummonAbility(bool bWasCancelled);
	void WriteBlackboardState(bool bHasSpawnedEnemies, int32 SpawnPhase);
	UBlackboardComponent* GetBlackboardComponentFromActorInfo() const;
};
