// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/EnemyGameplayAbility_SummonEnemies.h"
#include "AbilitySystem/AbilityTasks/AbilityTask_WaitSpawnEnemies.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/WarriorEnemyCharacter.h"
#include "WarriorGameplayTags.h"

UEnemyGameplayAbility_SummonEnemies::UEnemyGameplayAbility_SummonEnemies()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	EventTag = WarriorGameplayTags::Enemy_Event_SummonEnemies;
}

void UEnemyGameplayAbility_SummonEnemies::PostLoad()
{
	Super::PostLoad();

	if (!EventTag.IsValid())
	{
		EventTag = WarriorGameplayTags::Enemy_Event_SummonEnemies;
	}
}

void UEnemyGameplayAbility_SummonEnemies::PostInitProperties()
{
	Super::PostInitProperties();

	if (!EventTag.IsValid())
	{
		EventTag = WarriorGameplayTags::Enemy_Event_SummonEnemies;
	}
}

void UEnemyGameplayAbility_SummonEnemies::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FGameplayTag EventTagToUse = EventTag.IsValid() ? EventTag : WarriorGameplayTags::Enemy_Event_SummonEnemies;
	EventTag = EventTagToUse;

	if (!MontageToPlay)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FWarriorEnemySummonHealthConfig SelectedSummonConfig;
	if (!NativeSelectConfiguredEnemySummonConfigByCurrentHealth(SelectedSummonConfig, CurrentSummonPhase))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MontageToPlay
	);

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	SpawnEnemiesTask = UAbilityTask_WaitSpawnEnemies::WaitSpawnEnemyGroups(
		this,
		EventTagToUse,
		SelectedSummonConfig,
		GetAvatarActorFromActorInfo()->GetActorLocation(),
		RandomSpawnRadius
	);

	if (!SpawnEnemiesTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::HandleMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::HandleMontageInterruptedOrCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::HandleMontageInterruptedOrCancelled);

	SpawnEnemiesTask->OnSpawnFinished.AddDynamic(this, &ThisClass::HandleSpawnFinished);
	SpawnEnemiesTask->DidNotSpawn.AddDynamic(this, &ThisClass::HandleSpawnFailed);

	SpawnEnemiesTask->ReadyForActivation();
	MontageTask->ReadyForActivation();
}

void UEnemyGameplayAbility_SummonEnemies::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	MontageTask = nullptr;
	SpawnEnemiesTask = nullptr;
	CurrentSummonPhase = INDEX_NONE;
	bIsEndingAbility = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UEnemyGameplayAbility_SummonEnemies::HandleSpawnFinished(const TArray<AWarriorEnemyCharacter*>& SpawnedEnemies)
{
	MarkSummonFinished(true, SpawnedEnemies.Num());
	EndSummonAbility(false);
}

void UEnemyGameplayAbility_SummonEnemies::HandleSpawnFailed(const TArray<AWarriorEnemyCharacter*>& SpawnedEnemies)
{
	MarkSummonFinished(false, SpawnedEnemies.Num());
	EndSummonAbility(true);
}

void UEnemyGameplayAbility_SummonEnemies::HandleMontageEnded()
{
	if (SpawnEnemiesTask && SpawnEnemiesTask->HasReceivedGameplayEvent())
	{
		return;
	}

	EndSummonAbility(false);
}

void UEnemyGameplayAbility_SummonEnemies::HandleMontageInterruptedOrCancelled()
{
	EndSummonAbility(true);
}

void UEnemyGameplayAbility_SummonEnemies::MarkSummonFinished(bool bSuccess, int32 SpawnedEnemyCount)
{
	WriteBlackboardState(bSuccess && SpawnedEnemyCount > 0, CurrentSummonPhase);
	SpawnEnemiesTask = nullptr;
}

void UEnemyGameplayAbility_SummonEnemies::EndSummonAbility(bool bWasCancelled)
{
	if (bIsEndingAbility)
	{
		return;
	}

	bIsEndingAbility = true;
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, bWasCancelled);
}

void UEnemyGameplayAbility_SummonEnemies::WriteBlackboardState(bool bHasSpawnedEnemies, int32 SpawnPhase)
{
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponentFromActorInfo();
	if (!BlackboardComponent)
	{
		return;
	}

	if (!HasSpawnedEnemiesKeyName.IsNone())
	{
		BlackboardComponent->SetValueAsBool(HasSpawnedEnemiesKeyName, bHasSpawnedEnemies);
	}

	if (bWriteSummonPhaseToBlackboard && !SummonPhaseKeyName.IsNone() && SpawnPhase != INDEX_NONE)
	{
		BlackboardComponent->SetValueAsInt(SummonPhaseKeyName, SpawnPhase);
	}
}

UBlackboardComponent* UEnemyGameplayAbility_SummonEnemies::GetBlackboardComponentFromActorInfo() const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	APawn* AvatarPawn = Cast<APawn>(AvatarActor);
	if (!AvatarPawn)
	{
		return nullptr;
	}

	AAIController* AIController = AvatarPawn->GetController<AAIController>();
	return AIController ? AIController->GetBlackboardComponent() : nullptr;
}
