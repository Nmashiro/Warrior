// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/WarriorProjectileBase.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"

#include "WarriorDebugHelper.h"
AWarriorProjectileBase::AWarriorProjectileBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ProjectileCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ProjectileCollisionBox"));
	SetRootComponent(ProjectileCollisionBox);
	ProjectileCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	ProjectileCollisionBox->SetGenerateOverlapEvents(true);
	ProjectileCollisionBox->OnComponentHit.AddUniqueDynamic(this, &ThisClass::OnProjectileHit);
	ProjectileCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnProjectBeginOverlap);

	ProjectileNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileNiagaraComponent"));
	ProjectileNiagaraComponent->SetupAttachment(GetRootComponent());

	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->InitialSpeed = 700.f;
	ProjectileMovementComp->MaxSpeed = 900.f;
	ProjectileMovementComp->Velocity = FVector(1.f, 0.f, 0.f);
	ProjectileMovementComp->ProjectileGravityScale = 0.f;

	InitialLifeSpan = 4.f;
}


void AWarriorProjectileBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWarriorProjectileBase::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* HitPawn = Cast<APawn>(OtherActor);

	if (HitPawn)
	{
		if (!IsValidProjectileTarget(HitPawn))
		{
			return;
		}

		BP_OnSpawnProjectileHitFX(Hit.ImpactPoint);

		FGameplayEventData Data;
		Data.Instigator = this;
		Data.Target = HitPawn;
		HandleProjectileImpactOnPawn(HitPawn, Data);

		Destroy();
		return;
	}

	BP_OnSpawnProjectileHitFX(Hit.ImpactPoint);
	Destroy();
}

void AWarriorProjectileBase::OnProjectBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedActors.Contains(OtherActor))
	{
		return;
	}

	APawn* HitPawn = Cast<APawn>(OtherActor);
	if (!IsValidProjectileTarget(HitPawn))
	{
		return;
	}

	OverlappedActors.AddUnique(OtherActor);

	FGameplayEventData Data;
	if (ProjectileDamagePolicy == EProjectileDamagePolicy::OnHit)
	{
		Data.Instigator = this;
	}
	else
	{
		Data.Instigator = GetInstigator();
	}
	Data.Target = HitPawn;

	if (ProjectileDamagePolicy == EProjectileDamagePolicy::OnHit)
	{
		FVector HitLocation = GetActorLocation();
		if (bFromSweep)
		{
			HitLocation = FVector(SweepResult.ImpactPoint);
		}

		BP_OnSpawnProjectileHitFX(HitLocation);
		HandleProjectileImpactOnPawn(HitPawn, Data);
		Destroy();
		return;
	}

	HandleApplyProjectileDamage(HitPawn, Data);
}

bool AWarriorProjectileBase::IsValidProjectileTarget(APawn* InHitPawn) const
{
	APawn* InstigatorPawn = GetInstigator();

	return InstigatorPawn && InHitPawn && UWarriorFunctionLibrary::IsTargetPawnHostile(InstigatorPawn, InHitPawn);
}

void AWarriorProjectileBase::HandleProjectileImpactOnPawn(APawn* InHitPawn, const FGameplayEventData& InPayload)
{
	bool bIsValidBlock = false;
	const bool bIsPlayerBlocking = UWarriorFunctionLibrary::NativeDoesActorHaveTag(InHitPawn, WarriorGameplayTags::Player_Status_Blocking);

	if (bIsPlayerBlocking)
	{
		bIsValidBlock = UWarriorFunctionLibrary::IsValidBlock(this, InHitPawn);
	}

	if (bIsValidBlock)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			InHitPawn,
			WarriorGameplayTags::Player_Event_SuccessfulBlock,
			InPayload
		);
	}
	else
	{
		HandleApplyProjectileDamage(InHitPawn, InPayload);
	}
}

void AWarriorProjectileBase::HandleApplyProjectileDamage(APawn* InHitPawn, const FGameplayEventData& InPayload)
{
	checkf(ProjectileDamageEffectSpecHandle.IsValid(), TEXT("Forgot to assign a valid spec handle to the projectile: %s"), *GetActorNameOrLabel());

	const bool bWasApplied = UWarriorFunctionLibrary::ApplyGameplayEffectSpecHandleToTargetActor(GetInstigator(), InHitPawn, ProjectileDamageEffectSpecHandle);

	if (bWasApplied)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			InHitPawn,
			WarriorGameplayTags::Shared_Event_HitReact,
			InPayload
		);
	}
}



