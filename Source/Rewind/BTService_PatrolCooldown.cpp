// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_PatrolCooldown.h"
#include "RewindEnemyBaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTService_PatrolCooldown::UBTService_PatrolCooldown()
{
	NodeName = TEXT("Patrol Cooldown Timer");
	Interval = 0.2f; // ����I�Ƀ`�F�b�N
}

void UBTService_PatrolCooldown::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	
	float CooldownEndTime = BB->GetValueAsFloat("PatrolCooldownEndTime");

	// ���ݎ��Ԃ��擾
	UWorld* World = OwnerComp.GetWorld();
	if (!World) return;

	float CurrentTime = World->GetTimeSeconds();

	if (CurrentTime >= CooldownEndTime)
	{
		BB->SetValueAsBool("bCanPatrol", true);
	}
	//UE_LOG(LogTemp, Warning, TEXT("UBTService_PatrolCooldown: %f,%f"), CurrentTime, CooldownEndTime);
}