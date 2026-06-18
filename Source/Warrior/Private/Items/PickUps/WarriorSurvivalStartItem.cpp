// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/PickUps/WarriorSurvivalStartItem.h"
#include "Characters/WarriorHeroCharacter.h"
#include "Component/UI/HeroUIComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameModes/WarriorSurvivalGameMode.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

#define LOCTEXT_NAMESPACE "WarriorSurvivalStartItem"

AWarriorSurvivalStartItem::AWarriorSurvivalStartItem()
{
	static ConstructorHelpers::FObjectFinder<UParticleSystem> DefaultCascadeFireEffect(TEXT("/Game/InfinityBladeIceLands/Effects/FX_Ambient/Fire/P_Fire_Torch_01.P_Fire_Torch_01"));
	if (DefaultCascadeFireEffect.Succeeded())
	{
		CascadeFireEffectTemplate = DefaultCascadeFireEffect.Object;
	}

	DifficultyMessages =
	{
		{ EWarriorGameDifficulty::Easy, LOCTEXT("EasyStartMessageDefault", "The embers answer softly. Easy trial begins.") },
		{ EWarriorGameDifficulty::Normal, LOCTEXT("NormalStartMessageDefault", "The fire wakes. Normal trial begins.") },
		{ EWarriorGameDifficulty::Hard, LOCTEXT("HardStartMessageDefault", "The flames roar. Hard trial begins.") },
		{ EWarriorGameDifficulty::VeryHard, LOCTEXT("VeryHardStartMessageDefault", "The pyre screams. Very Hard trial begins.") }
	};
}

void AWarriorSurvivalStartItem::Interact(AWarriorHeroCharacter* InteractingHero, int32 ApplyLevel)
{
	if (!CanInteract() || bHasBeenUsed || !InteractingHero)
	{
		return;
	}

	AWarriorSurvivalGameMode* SurvivalGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AWarriorSurvivalGameMode>() : nullptr;
	if (!SurvivalGameMode || SurvivalGameMode->HasSurvivalGameStarted())
	{
		return;
	}

	bHasBeenUsed = true;
	SetCanInteract(false);
	CachedInteractingHero = InteractingHero;
	CachedSurvivalGameMode = SurvivalGameMode;

	const EWarriorGameDifficulty CurrentDifficulty = SurvivalGameMode->GetCurrentGameDifficulty();
	const FText DifficultyMessage = GetDifficultyMessage(CurrentDifficulty);
	const float TotalDisplayDuration = FirePitHideDelay + FireDissipateDelay;

	if (UHeroUIComponent* HeroUIComponent = InteractingHero->GetHeroUIComponent())
	{
		HeroUIComponent->OnSurvivalStartMessageRequested.Broadcast(DifficultyMessage, TotalDisplayDuration);
	}

	StartFireEffect();
	BP_OnSurvivalStartItemActivated(InteractingHero, DifficultyMessage);

	FTimerHandle HideFirePitTimerHandle;
	GetWorldTimerManager().SetTimer(
		HideFirePitTimerHandle,
		this,
		&ThisClass::HideFirePit,
		FirePitHideDelay,
		false
	);
}

FText AWarriorSurvivalStartItem::GetDifficultyMessage(EWarriorGameDifficulty Difficulty) const
{
	for (const FWarriorSurvivalStartDifficultyMessage& DifficultyMessage : DifficultyMessages)
	{
		if (DifficultyMessage.Difficulty == Difficulty && !DifficultyMessage.Message.IsEmpty())
		{
			return DifficultyMessage.Message;
		}
	}

	switch (Difficulty)
	{
	case EWarriorGameDifficulty::Easy:
		return LOCTEXT("EasyStartMessage", "The embers answer softly. Easy trial begins.");
	case EWarriorGameDifficulty::Normal:
		return LOCTEXT("NormalStartMessage", "The fire wakes. Normal trial begins.");
	case EWarriorGameDifficulty::Hard:
		return LOCTEXT("HardStartMessage", "The flames roar. Hard trial begins.");
	case EWarriorGameDifficulty::VeryHard:
		return LOCTEXT("VeryHardStartMessage", "The pyre screams. Very Hard trial begins.");
	default:
		return LOCTEXT("DefaultStartMessage", "The fire wakes. The trial begins.");
	}
}

void AWarriorSurvivalStartItem::StartFireEffect()
{
	if (UFXSystemComponent* FireEffectComponent = ResolveFireEffectComponentToActivate())
	{
		FireEffectComponent->SetHiddenInGame(false);
		FireEffectComponent->Activate(true);
		return;
	}

	USceneComponent* AttachComponent = GetRootComponent();

	if (CascadeFireEffectTemplate)
	{
		UParticleSystemComponent* CascadeFireEffectComponent = NewObject<UParticleSystemComponent>(this, TEXT("SpawnedSurvivalStartCascadeFireEffect"));

		if (CascadeFireEffectComponent)
		{
			CascadeFireEffectComponent->SetTemplate(CascadeFireEffectTemplate);
			CascadeFireEffectComponent->SetupAttachment(AttachComponent, FireEffectAttachSocketName);
			CascadeFireEffectComponent->SetRelativeTransform(FireEffectRelativeTransform);
			CascadeFireEffectComponent->RegisterComponent();
			CascadeFireEffectComponent->Activate(true);
			SpawnedFireEffectComponent = CascadeFireEffectComponent;
			return;
		}
	}

	if (FireEffectTemplate)
	{
		UNiagaraComponent* NiagaraFireEffectComponent = NewObject<UNiagaraComponent>(this, TEXT("SpawnedSurvivalStartNiagaraFireEffect"));

		if (NiagaraFireEffectComponent)
		{
			NiagaraFireEffectComponent->SetAsset(FireEffectTemplate);
			NiagaraFireEffectComponent->SetupAttachment(AttachComponent, FireEffectAttachSocketName);
			NiagaraFireEffectComponent->SetRelativeTransform(FireEffectRelativeTransform);
			NiagaraFireEffectComponent->RegisterComponent();
			NiagaraFireEffectComponent->Activate(true);
			SpawnedFireEffectComponent = NiagaraFireEffectComponent;
		}
	}
}

void AWarriorSurvivalStartItem::HideFirePitVisual()
{
	if (UPrimitiveComponent* ExplicitFirePitComponent = Cast<UPrimitiveComponent>(FirePitComponentToHide.GetComponent(this)))
	{
		ExplicitFirePitComponent->SetVisibility(false, false);
		ExplicitFirePitComponent->SetHiddenInGame(true, false);
		ExplicitFirePitComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return;
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents(PrimitiveComponents);

	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (IsAutoFirePitVisualComponent(PrimitiveComponent))
		{
			PrimitiveComponent->SetVisibility(false, false);
			PrimitiveComponent->SetHiddenInGame(true, false);
			PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AWarriorSurvivalStartItem::DissipateFireEffect()
{
	BP_OnSurvivalStartFireDissipating();

	if (UFXSystemComponent* FireEffectComponent = ResolveFireEffectComponentToActivate())
	{
		FireEffectComponent->Deactivate();
		return;
	}

	if (SpawnedFireEffectComponent)
	{
		SpawnedFireEffectComponent->Deactivate();
	}
}

UFXSystemComponent* AWarriorSurvivalStartItem::ResolveFireEffectComponentToActivate()
{
	if (UFXSystemComponent* ExplicitFireEffectComponent = Cast<UFXSystemComponent>(FireEffectComponentToActivate.GetComponent(this)))
	{
		return ExplicitFireEffectComponent;
	}

	TArray<UFXSystemComponent*> FireEffectComponents;
	GetComponents(FireEffectComponents);

	for (UFXSystemComponent* FireEffectComponent : FireEffectComponents)
	{
		if (FireEffectComponent && FireEffectComponent != SpawnedFireEffectComponent)
		{
			return FireEffectComponent;
		}
	}

	return nullptr;
}

bool AWarriorSurvivalStartItem::IsAutoFirePitVisualComponent(UPrimitiveComponent* PrimitiveComponent) const
{
	if (!PrimitiveComponent || PrimitiveComponent == PickUpCollisionSphere)
	{
		return false;
	}

	if (PrimitiveComponent == SpawnedFireEffectComponent || Cast<UFXSystemComponent>(PrimitiveComponent))
	{
		return false;
	}

	return PrimitiveComponent->IsVisible();
}

void AWarriorSurvivalStartItem::HideFirePit()
{
	HideFirePitVisual();
	BP_OnSurvivalStartFirePitHidden();

	DissipateFireEffect();

	FTimerHandle FinishActivationSequenceTimerHandle;
	GetWorldTimerManager().SetTimer(
		FinishActivationSequenceTimerHandle,
		this,
		&ThisClass::FinishActivationSequence,
		FireDissipateDelay,
		false
	);
}

void AWarriorSurvivalStartItem::FinishActivationSequence()
{
	BP_OnSurvivalStartFireDissipated();

	if (SpawnedFireEffectComponent && bDestroySpawnedFireEffectAfterDissipate)
	{
		SpawnedFireEffectComponent->DestroyComponent();
		SpawnedFireEffectComponent = nullptr;
	}

	if (AWarriorSurvivalGameMode* SurvivalGameMode = CachedSurvivalGameMode.Get())
	{
		SurvivalGameMode->StartSurvivalGame();
	}
}

#undef LOCTEXT_NAMESPACE
