// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HeroGameplayAbility_PickUpStones.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Characters/WarriorHeroCharacter.h"
#include "Items/PickUps/WarriorPickUpBase.h"
#include "Component/UI/HeroUIComponent.h"
#include "DrawDebugHelpers.h"

void UHeroGameplayAbility_PickUpStones::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	GetHeroUIComponentFromActorInfo()->OnStoneInteracted.Broadcast(true);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

}

void UHeroGameplayAbility_PickUpStones::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	GetHeroUIComponentFromActorInfo()->OnStoneInteracted.Broadcast(false);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

}

void UHeroGameplayAbility_PickUpStones::CollectInteractingItems()
{
	CollectedInteractingItems.Empty();

	TArray<AActor*> OverlappedActors;
	const FVector OverlapBoxCenter = GetHeroCharacterFromActorInfo()->GetActorLocation() - GetHeroCharacterFromActorInfo()->GetActorUpVector() * BoxTraceDistance * 0.5f;

	UKismetSystemLibrary::BoxOverlapActors(
		GetHeroCharacterFromActorInfo(),
		OverlapBoxCenter,
		TraceBoxSize / 2.f,
		StoneTraceChannel,
		AWarriorPickUpBase::StaticClass(),
		TArray<AActor*>(),
		OverlappedActors
	);

	if (bDrawDebugShage)
	{
		DrawDebugBox(
			GetWorld(),
			OverlapBoxCenter,
			TraceBoxSize / 2.f,
			FColor::Green,
			false,
			1.f
		);
	}

	for (AActor* OverlappedActor : OverlappedActors)
	{
		if (AWarriorPickUpBase* FoundInteractingItem = Cast<AWarriorPickUpBase>(OverlappedActor))
		{
			if (FoundInteractingItem->CanInteract())
			{
				CollectedInteractingItems.AddUnique(FoundInteractingItem);
			}
		}
	}

	if (CollectedInteractingItems.IsEmpty())
	{
		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
	}
}

void UHeroGameplayAbility_PickUpStones::InteractWithNearestItem()
{
	if (CollectedInteractingItems.IsEmpty())
	{
		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
		return;
	}

	AWarriorPickUpBase* NearestInteractingItem = nullptr;
	float NearestDistanceSquared = TNumericLimits<float>::Max();
	const FVector HeroLocation = GetHeroCharacterFromActorInfo()->GetActorLocation();

	for (AWarriorPickUpBase* CollectedInteractingItem : CollectedInteractingItems)
	{
		if (!CollectedInteractingItem || !CollectedInteractingItem->CanInteract())
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(HeroLocation, CollectedInteractingItem->GetActorLocation());
		if (DistanceSquared < NearestDistanceSquared)
		{
			NearestDistanceSquared = DistanceSquared;
			NearestInteractingItem = CollectedInteractingItem;
		}
	}

	if (NearestInteractingItem)
	{
		NearestInteractingItem->Interact(GetHeroCharacterFromActorInfo(), GetAbilityLevel());
	}
	else
	{
		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
	}
}

void UHeroGameplayAbility_PickUpStones::CollectStones()
{
	CollectInteractingItems();
}

void UHeroGameplayAbility_PickUpStones::ConsumeStones()
{
	InteractWithNearestItem();
}
