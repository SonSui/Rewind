// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RewindBaseCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "RewindEnemyBaseCharacter.generated.h"

class AAIController;

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle,        // 站立
	Patrolling,
	Chasing,
	Attacking,
	Stunned,
	Dead,
	Rewinding
};
/**
 * 敵キャラクターの基底クラス
 */
UCLASS()
class REWIND_API ARewindEnemyBaseCharacter : public ARewindBaseCharacter
{
	GENERATED_BODY()

public:
	ARewindEnemyBaseCharacter();

protected:
	virtual void BeginPlay() override;


public:


	// プレイヤーを視認した時に呼ばれる（AI感知から）
	UFUNCTION(BlueprintCallable, Category = "AI")
	virtual void OnPlayerSpotted(APawn* PlayerPawn);

	// ダメージを受けたときの処理（親からオーバーライド）
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	// 攻撃中か
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bIsAttacking = false;

	// 攻撃間隔（秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackCooldown = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DefaultAttackCooldown = 2.0f;

	// 攻撃タイマー
	FTimerHandle AttackCooldownTimer;

	// ========== 行動関数（BTTask・State管理用） ==========
	UFUNCTION(BlueprintCallable, Category = "Action")
	virtual bool EnterIdle(bool bShouldRestartBT = false) { return false; };

	UFUNCTION(BlueprintCallable, Category = "Action")
	virtual bool EnterPatrol() { return false; };

	UFUNCTION(BlueprintCallable, Category = "Action")
	virtual bool EnterChase(AActor* PlayerActor) { return false; };

	UFUNCTION(BlueprintCallable, Category = "Action")
	virtual bool EnterAttack() { return false; };

	UFUNCTION(BlueprintCallable, Category = "Action")
	virtual void EnterStun() {};

	UFUNCTION(BlueprintCallable, Category = "Action")
	virtual void EnterDeath() {};

	UFUNCTION(BlueprintCallable, Category = "Action")
	virtual bool MoveToLocation(const FVector& Location) { return false; };

	UFUNCTION(BlueprintCallable, Category = "Action")
	virtual void FaceTarget(const FVector& TargetLocation) {};

	UFUNCTION(BlueprintCallable, Category = "Action")
	virtual void TryTurnToPlayer() {};


	// ========== 補助関数 ==========
	UFUNCTION(BlueprintCallable, Category = "Helper")
	virtual bool CanAttack() const { return false; };

	UFUNCTION(BlueprintCallable, Category = "Helper")
	virtual bool IsInAttackRange(const FVector& PlayerLocation) const { return false; };

	UFUNCTION(BlueprintCallable, Category = "Helper")
	virtual bool IsPlayerVisible() const { return false; };

	UFUNCTION(BlueprintCallable, Category = "Helper")
	virtual void ResetToughness() {};

	UFUNCTION(BlueprintCallable, Category = "Helper")
	virtual void ResetStunTime() {};
	// 行動ロジックの一時停止 
	UFUNCTION(BlueprintCallable, Category = "BT Control")
	virtual void StopBTLogic(const FString& Reason) {};

	// 行動ロジックの再開 
	UFUNCTION(BlueprintCallable, Category = "BT Control")
	virtual void RestartBTLogic() {};
	UFUNCTION(BlueprintCallable, Category = "BT Control")
	virtual FVector GetSpawnPoint() const { return SpawnPoint; }

	virtual void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted) {};

	// ========== 行動フラグ（BTTask・State管理用） ==========
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
	bool bCanPatrol = true;  // パトロール可能かどうか

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
	bool bOverPatrolRange = false;  // パトロール範囲外かどうか

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
	FVector MoveTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
	AActor* TargetActor;  // 攻撃対象

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
	bool bPlayerDetected = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
	bool bPlayerInAttackRange = false;
	// ========== パトロール ==========


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grux|Patrol")
	float PatrolRadius = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grux|Patrol")
	float PatrolInterval = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grux|Patrol")
	float DefaultPatrolInterval = 5.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Patrol")
	FVector SpawnPoint;

	// ========== BehaviorTree ==========
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UBehaviorTree* BehaviorTreeAsset;
};