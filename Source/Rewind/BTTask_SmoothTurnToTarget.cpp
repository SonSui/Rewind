// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_SmoothTurnToTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "RewindEnemyBaseCharacter.h"

UBTTask_SmoothTurnToTarget::UBTTask_SmoothTurnToTarget()
{
	NodeName = TEXT("Smooth Turn To Target");
	bNotifyTick = true; 
}

EBTNodeResult::Type UBTTask_SmoothTurnToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	
	ARewindEnemyBaseCharacter* Grux = Cast<ARewindEnemyBaseCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Grux) return EBTNodeResult::Failed;

	UObject* TargetObj = OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor"));
	AActor* Target = Cast<AActor>(TargetObj);
	if (!Target) return EBTNodeResult::Failed;

	return EBTNodeResult::InProgress; // Tick
}

void UBTTask_SmoothTurnToTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ARewindEnemyBaseCharacter* Grux = Cast<ARewindEnemyBaseCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Grux) return;

	AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));
	if (!Target) return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	FVector ToTarget = Target->GetActorLocation() - Grux->GetActorLocation();
	ToTarget.Z = 0.0f;

	if (ToTarget.IsNearlyZero())
	{
		BB->SetValueAsBool(TEXT("bShouldTurnToPlayer"), false);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	FRotator Current = Grux->GetActorRotation();
	FRotator TargetRot = ToTarget.Rotation();
	FRotator NewRot = FMath::RInterpTo(Current, TargetRot, DeltaSeconds, TurnSpeed);

	Grux->SetActorRotation(NewRot);

	
	if (FMath::Abs((TargetRot - NewRot).Yaw) < AcceptableAngle)
	{
		BB->SetValueAsBool(TEXT("bShouldTurnToPlayer"), false);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

