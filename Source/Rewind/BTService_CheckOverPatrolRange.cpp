// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_CheckOverPatrolRange.h"
#include "RewindEnemyBaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTService_CheckOverPatrolRange::UBTService_CheckOverPatrolRange()
{
    NodeName = TEXT("Check Over Patrol Range");
    Interval = 0.5f;
}

void UBTService_CheckOverPatrolRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    ARewindEnemyBaseCharacter* Grux = Cast<ARewindEnemyBaseCharacter>(OwnerComp.GetAIOwner()->GetPawn());
    if (!Grux) return;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    FVector CurrentLocation = Grux->GetActorLocation();
    FVector SpawnLocation = Grux->GetSpawnPoint();

    float Distance = FVector::Dist2D(CurrentLocation, SpawnLocation);
    float PatrolRadius = Grux->PatrolRadius;

    bool bOver = Distance > (PatrolRadius * 1.2f); // 120%’´‚¦‚½‚ç–ß‚é”»’è
    BB->SetValueAsBool(TEXT("bOverPatrolRange"), bOver);
    //UE_LOG(LogTemp, Warning, TEXT("Grux: UBTService_CheckOverPatrolRange :%s"), bOver ? TEXT("true") : TEXT("false"));
}
