// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ReturnToSpawn.h"
#include "RewindEnemyBaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTTask_ReturnToSpawn::UBTTask_ReturnToSpawn()
{
    NodeName = TEXT("Return To Spawn");
}

EBTNodeResult::Type UBTTask_ReturnToSpawn::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ARewindEnemyBaseCharacter* Grux = Cast<ARewindEnemyBaseCharacter>(OwnerComp.GetAIOwner()->GetPawn());
    if (!Grux) return EBTNodeResult::Failed;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;

    const FVector SpawnPos = Grux->GetSpawnPoint();
    BB->SetValueAsVector(TEXT("TargetLocation"), SpawnPos);
    //UE_LOG(LogTemp, Warning, TEXT("Grux: UBTTask_ReturnToSpawn"));
    return EBTNodeResult::Succeeded;
}
