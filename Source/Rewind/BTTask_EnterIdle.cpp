// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_EnterIdle.h"
#include "RewindEnemyBaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTTask_EnterIdle::UBTTask_EnterIdle()
{
	NodeName = TEXT("Enter Idle");
}

EBTNodeResult::Type UBTTask_EnterIdle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ARewindEnemyBaseCharacter* Grux = Cast<ARewindEnemyBaseCharacter>(OwnerComp.GetAIOwner()->GetPawn());
    if (!Grux) return EBTNodeResult::Failed;
    
    if (!Grux->EnterIdle())
    {
        return EBTNodeResult::Failed;
    }
    //UE_LOG(LogTemp, Warning, TEXT("Grux: UBTTask_EnterIdle Succeeded"));
    // パトロールのクールダウンをリセット
    Grux->bCanPatrol = false;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsBool(TEXT("bCanPatrol"), false);
    }

    return EBTNodeResult::Succeeded;
}
