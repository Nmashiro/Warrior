#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckNavMeshStatus.generated.h"

UCLASS()
class WARRIOR_API UBTService_CheckNavMeshStatus : public UBTService
{
	GENERATED_BODY()

	UBTService_CheckNavMeshStatus();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector bNeedsRecoveryKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector RecoveryLocationKey;

	UPROPERTY(EditAnywhere, Category = "NavMesh", meta = (UIMin = 50.f, UIMax = 2000.f))
	float SearchRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = "NavMesh", meta = (UIMin = 10.f, UIMax = 500.f))
	float MaxOffMeshDistance = 100.f;
};
