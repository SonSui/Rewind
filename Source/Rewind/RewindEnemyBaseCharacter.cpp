// Fill out your copyright notice in the Description page of Project Settings.
// RewindEnemyBaseCharacter.cpp
#include "RewindEnemyBaseCharacter.h"
#include "AIController.h"           // AI�R���g���[���[����p
#include "BrainComponent.h"         // StopLogic()�Ȃǂ̗��p�ɕK�v
#include "TimerManager.h"           // �^�C�}�[�����p
#include "Animation/AnimInstance.h" // �A�j���[�V�����Đ��p
#include "Components/CapsuleComponent.h" // CapsuleComponent �g�p�̂��ߕK�v

ARewindEnemyBaseCharacter::ARewindEnemyBaseCharacter()
{
    PrimaryActorTick.bCanEverTick = false; // �f�t�H���g�ł�Tick�𖳌��Ɂi�K�v�ȓG�ɂ����L�����j
}

void ARewindEnemyBaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth; // �����̗͐ݒ�
}




// �v���C���[�𔭌��������̏����i���E�⋗�����o���ɌĂԑz��j
void ARewindEnemyBaseCharacter::OnPlayerSpotted(APawn* PlayerPawn)
{
    // ���ϐ��̐ݒ��A�ړ�����������i���̓��O�̂݁j
    UE_LOG(LogTemp, Warning, TEXT("Player spotted: %s"), *PlayerPawn->GetName());
}

// �_���[�W�����iUnreal�W���֐��̃I�[�o�[���C�h�j
float ARewindEnemyBaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* DamageInstigator, AActor* DamageCauser)
{
    // ���ۂɓK�p�����_���[�W�ʂ��v�Z�i�̗͈ȏ�̃_���[�W�͖����j
    const float DamageApplied = FMath::Min(CurrentHealth, DamageAmount);
    CurrentHealth -= DamageApplied;

    return DamageApplied;
}


