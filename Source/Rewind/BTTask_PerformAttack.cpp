// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_PerformAttack.h"
#include "RewindEnemyBaseCharacter.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_PerformAttack::UBTTask_PerformAttack()
{
	NodeName = TEXT("Perform Attack");
}

EBTNodeResult::Type UBTTask_PerformAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ARewindEnemyBaseCharacter* Grux = Cast<ARewindEnemyBaseCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Grux) return EBTNodeResult::Failed;

	APawn* Player = UGameplayStatics::GetPlayerPawn(Grux->GetWorld(), 0);
	if (!Player) return EBTNodeResult::Failed;

	if (!Grux->IsInAttackRange(Player->GetActorLocation()))
	{
		return EBTNodeResult::Failed;
	}
	bool res = Grux->EnterAttack();
	//UE_LOG(LogTemp, Warning, TEXT("Grux: UBTTask_PerformAttack %s"),res? TEXT("true") : TEXT("false"));
	if(res) return EBTNodeResult::Succeeded;
	else return EBTNodeResult::Failed;
}