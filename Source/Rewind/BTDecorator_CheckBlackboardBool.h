// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CheckBlackboardBool.generated.h"

/**
 * 
 */
UCLASS()
class REWIND_API UBTDecorator_CheckBlackboardBool : public UBTDecorator
{
	GENERATED_BODY()
public:
	UBTDecorator_CheckBlackboardBool();

protected:
	// îªíËèàóù
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector BoolKey;
};
