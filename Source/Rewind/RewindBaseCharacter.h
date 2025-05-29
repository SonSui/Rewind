// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RewindBaseCharacter.generated.h"

class UAnimMontage;


/**
 * 全キャラクター共通の基底クラス
 */
UCLASS()
class REWIND_API ARewindBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ARewindBaseCharacter();

protected:
	virtual void BeginPlay() override;

public:
	// ダメージを受けた時に呼び出される
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// 通常攻撃アニメーションを再生
	virtual void PlayAttackMontage();

	// 被撃アニメーションを再生
	virtual void PlayHitMontage();

	// 死亡アニメーションを再生 
	virtual void PlayDeathMontage();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetCurrentHealth() const { return CurrentHealth; }
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, Category = "Stats")
	bool IsDead() { return bIsDead; }
protected:
	//最大体力 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float MaxHealth = 100.0f;

	// 現在体力
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth = 100.0f;

	// 攻撃モンタージュ 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* AttackMontage;

	// 被撃モンタージュ 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* HitMontage;

	// 死亡モンタージュ 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* DeathMontage;

	// 死亡状態確認 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	bool bIsStunned = false;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* AttackSound;

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayAttackSound();
	
};
