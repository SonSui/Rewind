// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RewindBaseCharacter.generated.h"

class UAnimMontage;


/**
 * �S�L�����N�^�[���ʂ̊��N���X
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
	// �_���[�W���󂯂����ɌĂяo�����
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// �ʏ�U���A�j���[�V�������Đ�
	virtual void PlayAttackMontage();

	// �팂�A�j���[�V�������Đ�
	virtual void PlayHitMontage();

	// ���S�A�j���[�V�������Đ� 
	virtual void PlayDeathMontage();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetCurrentHealth() const { return CurrentHealth; }
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, Category = "Stats")
	bool IsDead() { return bIsDead; }
protected:
	//�ő�̗� 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float MaxHealth = 100.0f;

	// ���ݑ̗�
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth = 100.0f;

	// �U�������^�[�W�� 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* AttackMontage;

	// �팂�����^�[�W�� 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* HitMontage;

	// ���S�����^�[�W�� 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* DeathMontage;

	// ���S��Ԋm�F 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	bool bIsStunned = false;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* AttackSound;

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayAttackSound();
	
};
