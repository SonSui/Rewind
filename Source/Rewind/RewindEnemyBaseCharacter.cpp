// Fill out your copyright notice in the Description page of Project Settings.
// RewindEnemyBaseCharacter.cpp
#include "RewindEnemyBaseCharacter.h"
#include "AIController.h"           // AIコントローラー制御用
#include "BrainComponent.h"         // StopLogic()などの利用に必要
#include "TimerManager.h"           // タイマー処理用
#include "Animation/AnimInstance.h" // アニメーション再生用
#include "Components/CapsuleComponent.h" // CapsuleComponent 使用のため必要

ARewindEnemyBaseCharacter::ARewindEnemyBaseCharacter()
{
    PrimaryActorTick.bCanEverTick = false; // デフォルトではTickを無効に（必要な敵にだけ有効化）
}

void ARewindEnemyBaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth; // 初期体力設定
}




// プレイヤーを発見した時の処理（視界や距離検出時に呼ぶ想定）
void ARewindEnemyBaseCharacter::OnPlayerSpotted(APawn* PlayerPawn)
{
    // 黒板変数の設定や、移動処理を入れる（今はログのみ）
    UE_LOG(LogTemp, Warning, TEXT("Player spotted: %s"), *PlayerPawn->GetName());
}

// ダメージ処理（Unreal標準関数のオーバーライド）
float ARewindEnemyBaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* DamageInstigator, AActor* DamageCauser)
{
    // 実際に適用されるダメージ量を計算（体力以上のダメージは無効）
    const float DamageApplied = FMath::Min(CurrentHealth, DamageAmount);
    CurrentHealth -= DamageApplied;

    return DamageApplied;
}


