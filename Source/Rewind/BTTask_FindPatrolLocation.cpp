// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FindPatrolLocation.h"
#include "RewindEnemyBaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTTask_FindPatrolLocation::UBTTask_FindPatrolLocation()
{
	NodeName = TEXT("Find Patrol Location");
}

EBTNodeResult::Type UBTTask_FindPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ARewindEnemyBaseCharacter* Grux = Cast<ARewindEnemyBaseCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Grux) return EBTNodeResult::Failed;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	if (Grux->EnterPatrol())
	{
		// パトロール位置設定
		BB->SetValueAsVector(TEXT("TargetLocation"), Grux->MoveTargetLocation);

		// パトロールクールダウン
		BB->SetValueAsBool(TEXT("bCanPatrol"), false);

		// クールダウン終了時間を設定
		float CooldownDuration = Grux->PatrolInterval;
		float EndTime = Grux->GetWorld()->GetTimeSeconds() + CooldownDuration;
		BB->SetValueAsFloat(TEXT("PatrolCooldownEndTime"), EndTime);

		//UE_LOG(LogTemp, Warning, TEXT("UBTTask_FindPatrolLocation : Succeeded"));
		return EBTNodeResult::Succeeded;
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("UBTTask_FindPatrolLocation : Failed"));
		return EBTNodeResult::Failed;
	}
}