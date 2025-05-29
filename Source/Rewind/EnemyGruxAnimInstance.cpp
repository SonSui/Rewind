// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyGruxAnimInstance.h"
#include "EnemyGruxCharacter.h"

void UEnemyGruxAnimInstance::RestartBTree()
{
	if (AEnemyGruxCharacter* PC = Cast<AEnemyGruxCharacter>(TryGetPawnOwner()))
	{
		PC->RestartBTLogic();
	}
}
void UEnemyGruxAnimInstance::AnimeSpeedStop()
{

	if (AEnemyGruxCharacter* PC = Cast<AEnemyGruxCharacter>(TryGetPawnOwner()))
	{
		PC->GetMesh()->bPauseAnims = true; // アニメーションを停止
	}
}
void UEnemyGruxAnimInstance::EnemyAttack()
{
	if (AEnemyGruxCharacter* PC = Cast<AEnemyGruxCharacter>(TryGetPawnOwner()))
	{
		PC->CheckPlayerByOverlap();
	}
}