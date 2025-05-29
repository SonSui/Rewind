// Fill out your copyright notice in the Description page of Project Settings.


#include "RewindBaseCharacter.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"

ARewindBaseCharacter::ARewindBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ARewindBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 開始時に体力を最大に
	CurrentHealth = MaxHealth;
}

float ARewindBaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float DamageApplied = FMath::Min(CurrentHealth, DamageAmount);
	CurrentHealth -= DamageApplied;

	// 被撃アニメーション再生
	PlayHitMontage();

	if (CurrentHealth <= 0.0f && !bIsDead)
	{
		bIsDead = true;
		PlayDeathMontage();
	}

	return DamageApplied;
}

void ARewindBaseCharacter::PlayAttackMontage()
{
	if (AttackMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(AttackMontage);
		}
	}
}

void ARewindBaseCharacter::PlayHitMontage()
{
	if (HitMontage)
	{
		bIsStunned = true; // スタン状態にする
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(HitMontage);

			// Stun モーション終わったらスタン解除
			FOnMontageEnded OnEnd;
			OnEnd.BindLambda([this](UAnimMontage* Montage, bool bInterrupted) {
				bIsStunned = false;
				});
			AnimInstance->Montage_SetEndDelegate(OnEnd, HitMontage);
		}
	}
}

void ARewindBaseCharacter::PlayDeathMontage()
{
	if (DeathMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(DeathMontage);
		}
	}
}
void ARewindBaseCharacter::PlayAttackSound()
{
	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
	}
}
