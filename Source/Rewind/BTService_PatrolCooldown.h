// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_PatrolCooldown.generated.h"

/**
 * 
 */
UCLASS()
class REWIND_API UBTService_PatrolCooldown : public UBTService_BlackboardBase
{
	GENERATED_BODY()
public:
    UBTService_PatrolCooldown();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

};
