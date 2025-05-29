// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_SetChaseTarget.h"
#include "RewindEnemyBaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTTask_SetChaseTarget::UBTTask_SetChaseTarget()
{
    NodeName = TEXT("Set Chase Target");
}

EBTNodeResult::Type UBTTask_SetChaseTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ARewindEnemyBaseCharacter* Grux = Cast<ARewindEnemyBaseCharacter>(OwnerComp.GetAIOwner()->GetPawn());
    if (!Grux) return EBTNodeResult::Failed;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;

    AActor* Player = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
    if (!Player) return EBTNodeResult::Failed;

    if (Grux->EnterChase(Player))
    {
        BB->SetValueAsVector(TEXT("TargetLocation"), Grux->MoveTargetLocation);
        //UE_LOG(LogTemp, Warning, TEXT("Grux: UBTTask_SetChaseTarget succeeded"));
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}
