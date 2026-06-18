// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/PickUps/WarriorPickUpBase.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Characters/WarriorHeroCharacter.h"
#include "Components/SphereComponent.h"
#include "WarriorGameplayTags.h"
// Sets default values
AWarriorPickUpBase::AWarriorPickUpBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PickUpCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickUpCollisionSphere"));
	SetRootComponent(PickUpCollisionSphere);
	PickUpCollisionSphere->InitSphereRadius(50.f);
	PickUpCollisionSphere->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnPickUpCollisionSphereBeginOverlap);
	PickUpCollisionSphere->OnComponentEndOverlap.AddUniqueDynamic(this, &ThisClass::OnPickUpCollisionSphereEndOverlap);
}

void AWarriorPickUpBase::Interact(AWarriorHeroCharacter* InteractingHero, int32 ApplyLevel)
{
}

void AWarriorPickUpBase::SetCanInteract(bool bInCanInteract)
{
	bCanInteract = bInCanInteract;

	if (!bCanInteract)
	{
		TArray<AActor*> OverlappingActors;
		PickUpCollisionSphere->GetOverlappingActors(OverlappingActors, AWarriorHeroCharacter::StaticClass());

		for (AActor* OverlappingActor : OverlappingActors)
		{
			if (AWarriorHeroCharacter* OverlappingHero = Cast<AWarriorHeroCharacter>(OverlappingActor))
			{
				OverlappingHero->RemoveOverlappingInteractingItem(this);
			}
		}
	}
}

void AWarriorPickUpBase::OnPickUpCollisionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bCanInteract)
	{
		return;
	}

	if (AWarriorHeroCharacter* OverlappedHeroCharacter = Cast<AWarriorHeroCharacter>(OtherActor))
	{
		OverlappedHeroCharacter->AddOverlappingInteractingItem(this);
		OverlappedHeroCharacter->GetWarriorAbilitySystemComponent()->TryActivateAbilityByTag(WarriorGameplayTags::Player_Ability_PickUp_Stones);
		BP_OnPickUpInteractionAvailable(OverlappedHeroCharacter, true);
	}
}

void AWarriorPickUpBase::OnPickUpCollisionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AWarriorHeroCharacter* OverlappedHeroCharacter = Cast<AWarriorHeroCharacter>(OtherActor))
	{
		OverlappedHeroCharacter->RemoveOverlappingInteractingItem(this);
		BP_OnPickUpInteractionAvailable(OverlappedHeroCharacter, false);
	}
}
