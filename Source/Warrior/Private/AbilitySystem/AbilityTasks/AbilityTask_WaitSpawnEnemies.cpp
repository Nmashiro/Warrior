// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AbilityTasks/AbilityTask_WaitSpawnEnemies.h"
#include "AbilitySystem/Abilities/WarriorEnemyGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Engine/AssetManager.h"
#include "NavigationSystem.h"
#include "Characters/WarriorEnemyCharacter.h"

#include "WarriorDebugHelper.h"
UAbilityTask_WaitSpawnEnemies* UAbilityTask_WaitSpawnEnemies::WaitSpawnEnemies(UGameplayAbility* OwningAbility, FGameplayTag EventTag, TSoftClassPtr<AWarriorEnemyCharacter> SoftEnemyClassToSpawn, int32 NumToSpawn, const FVector& SpawnOrigin, float RandomSpawnRadius)
{
	UAbilityTask_WaitSpawnEnemies* Node = NewAbilityTask<UAbilityTask_WaitSpawnEnemies>(OwningAbility);
	Node->CachedEventTag = EventTag;
	Node->CachedSoftEnemyClassToSpawn = SoftEnemyClassToSpawn;
	Node->CachedNumToSpawn = NumToSpawn;
	Node->CachedSpawnOrigin = SpawnOrigin;
	Node->CachedRandomSpawnRadius = RandomSpawnRadius;

	return Node;
}

UAbilityTask_WaitSpawnEnemies* UAbilityTask_WaitSpawnEnemies::WaitSpawnEnemyGroups(UGameplayAbility* OwningAbility, FGameplayTag EventTag, const FWarriorEnemySummonHealthConfig& EnemySummonConfig, const FVector& SpawnOrigin, float RandomSpawnRadius)
{
	UAbilityTask_WaitSpawnEnemies* Node = NewAbilityTask<UAbilityTask_WaitSpawnEnemies>(OwningAbility);
	Node->CachedEventTag = EventTag;
	Node->CachedEnemySummonInfos = EnemySummonConfig.EnemiesToSpawn;
	Node->CachedSpawnOrigin = SpawnOrigin;
	Node->CachedRandomSpawnRadius = RandomSpawnRadius;

	return Node;
}

void UAbilityTask_WaitSpawnEnemies::Activate()
{
	FGameplayEventMulticastDelegate& Delegate = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(CachedEventTag);

	DelegateHandle = Delegate.AddUObject(this, &ThisClass::OnGameplayEventReceived);
}

void UAbilityTask_WaitSpawnEnemies::OnDestroy(bool bInOwnerFinished)
{
	FGameplayEventMulticastDelegate& Delegate = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(CachedEventTag);

	Delegate.Remove(DelegateHandle);

	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_WaitSpawnEnemies::OnGameplayEventReceived(const FGameplayEventData* InPayload)
{
	bHasReceivedGameplayEvent = true;

	if (CachedEnemySummonInfos.IsEmpty())
	{
		FWarriorEnemySummonHealthConfig SelectedSummonConfig;
		if (UWarriorEnemyGameplayAbility* EnemyGameplayAbility = Cast<UWarriorEnemyGameplayAbility>(Ability))
		{
			if (EnemyGameplayAbility->NativeSelectConfiguredEnemySummonConfigByCurrentHealth(SelectedSummonConfig))
			{
				CachedEnemySummonInfos = SelectedSummonConfig.EnemiesToSpawn;
			}
		}
	}

	if (!CachedEnemySummonInfos.IsEmpty())
	{
		TArray<FSoftObjectPath> EnemyClassPathsToLoad;

		for (const FWarriorEnemySummonInfo& EnemySummonInfo : CachedEnemySummonInfos)
		{
			if (!EnemySummonInfo.IsValid())
			{
				continue;
			}

			EnemyClassPathsToLoad.AddUnique(EnemySummonInfo.SoftEnemyClassToSpawn.ToSoftObjectPath());
		}

		if (EnemyClassPathsToLoad.IsEmpty())
		{
			if (ShouldBroadcastAbilityTaskDelegates())
			{
				DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
			}

			EndTask();
			return;
		}

		UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
			EnemyClassPathsToLoad,
			FStreamableDelegate::CreateUObject(this, &ThisClass::OnEnemyGroupClassLoaded)
		);

		return;
	}

	if (ensure(!CachedSoftEnemyClassToSpawn.IsNull()))
	{
		UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
			CachedSoftEnemyClassToSpawn.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &ThisClass::OnEnemyClassLoaded)
		);
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
		}

		EndTask();
	}
}

void UAbilityTask_WaitSpawnEnemies::OnEnemyClassLoaded()
{
	UClass* LoadClass = CachedSoftEnemyClassToSpawn.Get();

	if (!LoadClass || !GetWorld())
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
		}

		EndTask();
		return;
	}

	TArray<AWarriorEnemyCharacter*> SpawnedEnemies;

	for (int32 i = 0; i < CachedNumToSpawn; i++)
	{
		AWarriorEnemyCharacter* SpawnedEnemy = SpawnEnemyFromLoadedClass(LoadClass, CachedSpawnOrigin, CachedRandomSpawnRadius);
		if (SpawnedEnemy)
		{
			SpawnedEnemies.Add(SpawnedEnemy);
		}
	}

	BroadcastSpawnResult(SpawnedEnemies);

	EndTask();
}

void UAbilityTask_WaitSpawnEnemies::OnEnemyGroupClassLoaded()
{
	SpawnLoadedEnemyGroups();
}

void UAbilityTask_WaitSpawnEnemies::SpawnLoadedEnemyGroups()
{
	if (!GetWorld())
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
		}

		EndTask();
		return;
	}

	TArray<AWarriorEnemyCharacter*> SpawnedEnemies;

	for (const FWarriorEnemySummonInfo& EnemySummonInfo : CachedEnemySummonInfos)
	{
		if (!EnemySummonInfo.IsValid())
		{
			continue;
		}

		UClass* LoadedEnemyClass = EnemySummonInfo.SoftEnemyClassToSpawn.Get();
		if (!LoadedEnemyClass)
		{
			continue;
		}

		for (int32 i = 0; i < EnemySummonInfo.NumToSpawn; i++)
		{
			AWarriorEnemyCharacter* SpawnedEnemy = SpawnEnemyFromLoadedClass(LoadedEnemyClass, CachedSpawnOrigin, CachedRandomSpawnRadius);
			if (SpawnedEnemy)
			{
				SpawnedEnemies.Add(SpawnedEnemy);
			}
		}
	}

	BroadcastSpawnResult(SpawnedEnemies);

	EndTask();
}

AWarriorEnemyCharacter* UAbilityTask_WaitSpawnEnemies::SpawnEnemyFromLoadedClass(UClass* LoadedEnemyClass, const FVector& SpawnOrigin, float RandomSpawnRadius)
{
	UWorld* World = GetWorld();
	if (!LoadedEnemyClass || !World || !AbilitySystemComponent.IsValid() || !AbilitySystemComponent->GetAvatarActor())
	{
		return nullptr;
	}

	FVector RandomLocation = SpawnOrigin;
	UNavigationSystemV1::K2_GetRandomReachablePointInRadius(this, SpawnOrigin, RandomLocation, RandomSpawnRadius);

	RandomLocation += FVector(0.f, 0.f, 150.f);

	const FRotator SpawnFacingRotation = AbilitySystemComponent->GetAvatarActor()->GetActorForwardVector().ToOrientationRotator();

	FActorSpawnParameters SpawnParam;
	SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	return World->SpawnActor<AWarriorEnemyCharacter>(LoadedEnemyClass, RandomLocation, SpawnFacingRotation, SpawnParam);
}

void UAbilityTask_WaitSpawnEnemies::BroadcastSpawnResult(const TArray<AWarriorEnemyCharacter*>& SpawnedEnemies)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		if (!SpawnedEnemies.IsEmpty())
		{
			OnSpawnFinished.Broadcast(SpawnedEnemies);
		}
		else
		{
			DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
		}
	}
}
