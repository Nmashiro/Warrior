// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WarriorPickUpBase.generated.h"

class AWarriorHeroCharacter;
class USphereComponent;

UCLASS()
class WARRIOR_API AWarriorPickUpBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWarriorPickUpBase();

	UFUNCTION(BlueprintCallable, Category = "Pick Up Interaction")
	virtual void Interact(AWarriorHeroCharacter* InteractingHero, int32 ApplyLevel);

	UFUNCTION(BlueprintPure, Category = "Pick Up Interaction")
	FORCEINLINE bool CanInteract() const { return bCanInteract; }

	UFUNCTION(BlueprintCallable, Category = "Pick Up Interaction")
	void SetCanInteract(bool bInCanInteract);

protected:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Pick Up Interaction")
	USphereComponent* PickUpCollisionSphere;

	UFUNCTION()
	virtual void OnPickUpCollisionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnPickUpCollisionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Pick Up Interaction Available"))
	void BP_OnPickUpInteractionAvailable(AWarriorHeroCharacter* InteractingHero, bool bIsAvailable);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pick Up Interaction")
	bool bCanInteract = true;

};
