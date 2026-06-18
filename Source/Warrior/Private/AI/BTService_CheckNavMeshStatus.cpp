#include "AI/BTService_CheckNavMeshStatus.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "GameFramework/Pawn.h"

UBTService_CheckNavMeshStatus::UBTService_CheckNavMeshStatus()
{
	NodeName = TEXT("Check NavMesh Status");

	INIT_SERVICE_NODE_NOTIFY_FLAGS();

	Interval = 0.1f;
	RandomDeviation = 0.f;

	bNeedsRecoveryKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, bNeedsRecoveryKey));
	RecoveryLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, RecoveryLocationKey));
}

void UBTService_CheckNavMeshStatus::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		bNeedsRecoveryKey.ResolveSelectedKey(*BBAsset);
		RecoveryLocationKey.ResolveSelectedKey(*BBAsset);
	}
}

FString UBTService_CheckNavMeshStatus::GetStaticDescription() const
{
	return FString::Printf(TEXT("Check if pawn is on NavMesh. Sets %s and %s if off-mesh"),
		*bNeedsRecoveryKey.SelectedKeyName.ToString(),
		*RecoveryLocationKey.SelectedKeyName.ToString());
}

void UBTService_CheckNavMeshStatus::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* OwningPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (!OwningPawn)
	{
		return;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return;
	}

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys)
	{
		return;
	}

	const FVector PawnLocation = OwningPawn->GetActorLocation();

	FNavLocation NavLocation;
	const bool bProjected = NavSys->ProjectPointToNavigation(PawnLocation, NavLocation, FVector(SearchRadius));

	if (!bProjected)
	{
		Blackboard->SetValueAsBool(bNeedsRecoveryKey.SelectedKeyName, true);
		Blackboard->SetValueAsVector(RecoveryLocationKey.SelectedKeyName, PawnLocation);
		return;
	}

	const float DistanceOffMesh = FVector::Dist(PawnLocation, NavLocation.Location);

	if (DistanceOffMesh > MaxOffMeshDistance)
	{
		Blackboard->SetValueAsBool(bNeedsRecoveryKey.SelectedKeyName, true);
		Blackboard->SetValueAsVector(RecoveryLocationKey.SelectedKeyName, NavLocation.Location);
	}
	else
	{
		Blackboard->SetValueAsBool(bNeedsRecoveryKey.SelectedKeyName, false);
	}
}
