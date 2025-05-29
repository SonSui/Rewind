// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyGruxAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class REWIND_API UEnemyGruxAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
	float Speed;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
	float moveAnimeSpeed;

	UFUNCTION(BlueprintCallable, Category = "AI")
	void RestartBTree();
	UFUNCTION(BlueprintCallable, Category = "Anime")
	void AnimeSpeedStop();
	UFUNCTION(BlueprintCallable, Category = "Hit")
	void EnemyAttack();
	
};
