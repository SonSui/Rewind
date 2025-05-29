// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_CheckAttackRange.h"
#include "RewindEnemyBaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"

UBTService_CheckAttackRange::UBTService_CheckAttackRange()
{
	NodeName = TEXT("Check Attack Range");
	Interval = 0.2f;
}

void UBTService_CheckAttackRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	ARewindEnemyBaseCharacter* Grux = Cast<ARewindEnemyBaseCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Grux) return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
	if (!Target)
	{
		BB->SetValueAsBool(TEXT("bPlayerInAttackRange"), false);
		return;
	}

	bool bInRange = Grux->IsInAttackRange(Target->GetActorLocation());
	BB->SetValueAsBool(TEXT("bPlayerInAttackRange"), bInRange);
	BB->SetValueAsBool(TEXT("bShouldTurnToPlayer"), bInRange);
	//UE_LOG(LogTemp, Warning, TEXT("Grux: UBTService_CheckAttackRange : %s"), bInRange ? TEXT("true") : TEXT("false"));
}
