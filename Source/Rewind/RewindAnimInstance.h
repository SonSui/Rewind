// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RewindAnimInstance.generated.h"

/**
 * Player Animation Blueprint Class
 */
UCLASS()
class REWIND_API URewindAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	// 入力ベクトル 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
	FVector2D InputVector;

	// 移動速度 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
	float Speed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
	float moveAnimeSpeed;

	UFUNCTION(BlueprintCallable, Category = "Combo")
	void ComboWindowCheck();
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void ComboWindowEnd();
	UFUNCTION(BlueprintCallable, Category = "Defence")
	void DefenceStop();
	UFUNCTION(BlueprintCallable, Category = "Defence")
	void DefenceContinue();
	UFUNCTION(BlueprintCallable, Category = "Attack")
	void TriggerAttack();
	UFUNCTION(BlueprintCallable, Category = "Hit")
	void HitStartRewind();

	UFUNCTION(BlueprintCallable, Category = "Hit")
	void PlayerDead();
	
};
