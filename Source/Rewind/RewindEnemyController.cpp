// Fill out your copyright notice in the Description page of Project Settings.


#include "RewindEnemyController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"

ARewindEnemyController::ARewindEnemyController()
{
	
}

void ARewindEnemyController::BeginPlay()
{
	Super::BeginPlay();

	if (UseBlackboard(BlackboardAsset, MyBlackboard))
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}
}