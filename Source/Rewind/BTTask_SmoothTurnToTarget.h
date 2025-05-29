// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SmoothTurnToTarget.generated.h"

/**
 * 
 */
UCLASS()
class REWIND_API UBTTask_SmoothTurnToTarget : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_SmoothTurnToTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnGameplayTaskActivated(UGameplayTask& Task) override {}

protected:
	UPROPERTY(EditAnywhere, Category = "Turn")
	float TurnSpeed = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Turn")
	float AcceptableAngle = 5.0f;
};
