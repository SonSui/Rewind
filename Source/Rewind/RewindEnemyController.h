// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RewindEnemyController.generated.h"

/**
 * 
 */
UCLASS()
class REWIND_API ARewindEnemyController : public AAIController
{
	GENERATED_BODY()
public:
	ARewindEnemyController();

protected:
	virtual void BeginPlay() override;

	// çsìÆÇ…égÇ§BehaviorTree
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UBehaviorTree* BehaviorTreeAsset;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UBlackboardData* BlackboardAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UBlackboardComponent* MyBlackboard;
	
};
