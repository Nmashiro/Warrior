// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Items/PickUps/WarriorPickUpBase.h"
#include "WarriorTypes/WarriorEnumTypes.h"
#include "WarriorSurvivalStartItem.generated.h"

class AWarriorHeroCharacter;
class AWarriorSurvivalGameMode;
class UFXSystemComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UParticleSystem;
class UPrimitiveComponent;

USTRUCT(BlueprintType)
struct FWarriorSurvivalStartDifficultyMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EWarriorGameDifficulty Difficulty = EWarriorGameDifficulty::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Message;
};

/**
 * Place this in a survival level to let the player decide when the first wave starts.
 */
UCLASS()
class WARRIOR_API AWarriorSurvivalStartItem : public AWarriorPickUpBase
{
	GENERATED_BODY()

public:
	AWarriorSurvivalStartItem();

	virtual void Interact(AWarriorHeroCharacter* InteractingHero, int32 ApplyLevel) override;

	UFUNCTION(BlueprintPure, Category = "Survival")
	FORCEINLINE bool HasBeenUsed() const { return bHasBeenUsed; }

protected:
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Survival Start Item Activated"))
	void BP_OnSurvivalStartItemActivated(AWarriorHeroCharacter* InteractingHero, const FText& DifficultyMessage);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Survival Start Fire Pit Hidden"))
	void BP_OnSurvivalStartFirePitHidden();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Survival Start Fire Dissipating"))
	void BP_OnSurvivalStartFireDissipating();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Survival Start Fire Dissipated"))
	void BP_OnSurvivalStartFireDissipated();

private:
	FText GetDifficultyMessage(EWarriorGameDifficulty Difficulty) const;
	void StartFireEffect();
	void HideFirePitVisual();
	void DissipateFireEffect();
	UFXSystemComponent* ResolveFireEffectComponentToActivate();
	bool IsAutoFirePitVisualComponent(UPrimitiveComponent* PrimitiveComponent) const;
	void HideFirePit();
	void FinishActivationSequence();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
	bool bHasBeenUsed = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Activation", meta = (AllowPrivateAccess = "true"))
	float FirePitHideDelay = 3.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Activation", meta = (AllowPrivateAccess = "true"))
	float FireDissipateDelay = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Activation", meta = (AllowPrivateAccess = "true"))
	TArray<FWarriorSurvivalStartDifficultyMessage> DifficultyMessages;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Visuals", meta = (AllowPrivateAccess = "true"))
	FComponentReference FirePitComponentToHide;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Visuals", meta = (AllowPrivateAccess = "true"))
	FComponentReference FireEffectComponentToActivate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Visuals", meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* FireEffectTemplate = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Visuals", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* CascadeFireEffectTemplate = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Visuals", meta = (AllowPrivateAccess = "true"))
	FName FireEffectAttachSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Visuals", meta = (AllowPrivateAccess = "true"))
	FTransform FireEffectRelativeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Visuals", meta = (AllowPrivateAccess = "true"))
	bool bDestroySpawnedFireEffectAfterDissipate = true;

	UPROPERTY()
	TWeakObjectPtr<AWarriorHeroCharacter> CachedInteractingHero;

	UPROPERTY()
	TWeakObjectPtr<AWarriorSurvivalGameMode> CachedSurvivalGameMode;

	UPROPERTY()
	UFXSystemComponent* SpawnedFireEffectComponent = nullptr;
};
