// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckOverPatrolRange.generated.h"

/**
 * 
 */
UCLASS()
class REWIND_API UBTService_CheckOverPatrolRange : public UBTService
{
	GENERATED_BODY()
public:
	UBTService_CheckOverPatrolRange();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
