// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ReturnToSpawn.generated.h"

/**
 * 
 */
UCLASS()
class REWIND_API UBTTask_ReturnToSpawn : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_ReturnToSpawn();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
