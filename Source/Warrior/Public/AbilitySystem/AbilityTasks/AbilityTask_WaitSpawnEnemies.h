// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "AbilityTask_WaitSpawnEnemies.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitSpawnEnemiesDelegate, const TArray<AWarriorEnemyCharacter*>&, SpawnedEnemies);

class AWarriorEnemyCharacter;
/**
 * 
 */
UCLASS()
class WARRIOR_API UAbilityTask_WaitSpawnEnemies : public UAbilityTask
{
	GENERATED_BODY()


public:
	UFUNCTION(BlueprintCallable, Category = "Warrior|AbilityTasks", meta = (DisplayName = "Wait Gameplay Event And Spawn Enemies", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", NumToSpawn = "1", RandomSpawnRadius = "200"))
	static UAbilityTask_WaitSpawnEnemies* WaitSpawnEnemies(
		UGameplayAbility* OwningAbility,
		FGameplayTag EventTag,
		TSoftClassPtr<AWarriorEnemyCharacter> SoftEnemyClassToSpawn,
		int32 NumToSpawn,
		const FVector& SpawnOrigin,
		float RandomSpawnRadius);

	UFUNCTION(BlueprintCallable, Category = "Warrior|AbilityTasks", meta = (DisplayName = "Wait Gameplay Event And Spawn Enemy Groups", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", RandomSpawnRadius = "200"))
	static UAbilityTask_WaitSpawnEnemies* WaitSpawnEnemyGroups(
		UGameplayAbility* OwningAbility,
		FGameplayTag EventTag,
		const FWarriorEnemySummonHealthConfig& EnemySummonConfig,
		const FVector& SpawnOrigin,
		float RandomSpawnRadius);

	UPROPERTY(BlueprintAssignable)
	FWaitSpawnEnemiesDelegate OnSpawnFinished;

	UPROPERTY(BlueprintAssignable)
	FWaitSpawnEnemiesDelegate DidNotSpawn;

	//~ Begin UGameplayTask Interface
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
	//~ End UGameplayTask Interface

	bool HasReceivedGameplayEvent() const { return bHasReceivedGameplayEvent; }

private:
	FGameplayTag CachedEventTag;
	TSoftClassPtr<AWarriorEnemyCharacter> CachedSoftEnemyClassToSpawn;
	int32 CachedNumToSpawn = 0;
	TArray<FWarriorEnemySummonInfo> CachedEnemySummonInfos;
	FVector CachedSpawnOrigin;
	float CachedRandomSpawnRadius = 0.f;
	FDelegateHandle DelegateHandle;
	bool bHasReceivedGameplayEvent = false;

	void OnGameplayEventReceived(const FGameplayEventData* InPayload);
	void OnEnemyClassLoaded();
	void OnEnemyGroupClassLoaded();
	void SpawnLoadedEnemyGroups();
	AWarriorEnemyCharacter* SpawnEnemyFromLoadedClass(UClass* LoadedEnemyClass, const FVector& SpawnOrigin, float RandomSpawnRadius);
	void BroadcastSpawnResult(const TArray<AWarriorEnemyCharacter*>& SpawnedEnemies);
};
