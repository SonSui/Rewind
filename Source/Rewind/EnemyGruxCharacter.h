// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RewindEnemyBaseCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "NiagaraSystem.h"
#include "IRewindable.h"
#include "RingBuffer.h"

#include "EnemyGruxCharacter.generated.h"

/**
 * 
 */

USTRUCT()
struct FGruxSnapshot
{
	GENERATED_BODY()

	UPROPERTY() FVector_NetQuantize Location;
	UPROPERTY() FRotator Rotation;
	UPROPERTY() FName   MontageName;
	UPROPERTY() float MontagePosition = 0.0f;

	UPROPERTY() float Speed;
	UPROPERTY() float MoveAnimeSpeed = 0.0f;

	UPROPERTY() EEnemyState State = EEnemyState::Idle;
	UPROPERTY() float AttackCooldown = 0.0f;
	UPROPERTY() int32 Toughness = 0;
	UPROPERTY() bool bStunable = false;
	UPROPERTY() float TimeStamp = 0.0f;

};

UCLASS()
class REWIND_API AEnemyGruxCharacter : public ARewindEnemyBaseCharacter, public IRewindable
{
	GENERATED_BODY()
public:
	AEnemyGruxCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	
	

	// ========== ステータス ==========

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grux|Stats")
	float MoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grux|Stats")
	float MaxMoveSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grux|Stats")
	float RotationSpeed = 360.0f;

	// ========== 視野 ==========
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grux|Sight")
	float SightAngle = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grux|Sight")
	float SightRadius = 800.0f;

	// ========== 攻撃 ==========
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grux|Combat")
	float AttackRange = 250.0f;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* HitSparkEffect;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CheckPlayerByOverlap(float Radius = 250.0f, float HalfAngleDegrees = 60.0f, float MaxHeightDiff = 100.0f);

	// ========== 被弾・硬直 ==========
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grux|Stun")
	float StunTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grux|Stun")
	float DefaultStunTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grux|Stun")
	int32 Toughness = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grux|Stun")
	int32 DefaultToughness = 3;

	


	
	

	// ========== アニメーション ==========
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grux|Animation")
	UAnimMontage* AttackPreMontage;

	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grux|Animation")
	UAnimMontage* AttackMontage2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grux|Animation")
	UAnimMontage* StunMontage;

	

	// ========== 行動関数（BTTask・State管理用） ==========
	
	bool EnterIdle(bool bShouldRestartBT = false) override;

	bool EnterPatrol() override;

	bool EnterChase(AActor* PlayerActor) override;

	bool EnterAttack() override;
	UFUNCTION(BlueprintCallable, Category = "Grux|Action")
	void StartAttack1();
	UFUNCTION(BlueprintCallable, Category = "Grux|Action")
	void StartAttack2();

	void EnterStun() override;

	void EnterDeath() override;

	bool MoveToLocation(const FVector& Location) override;

	void FaceTarget(const FVector& TargetLocation) override;

	void TryTurnToPlayer() override;

	// ========== 補助関数 ==========
	bool CanAttack() const override;

	bool IsInAttackRange(const FVector& PlayerLocation) const override;

	bool IsPlayerVisible() const override;

	void ResetToughness() override;

	void ResetStunTime() override;
	// 行動ロジックの一時停止 
	void StopBTLogic(const FString& Reason) override;

	// 行動ロジックの再開 
	void RestartBTLogic() override;
	
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted) override;




	// ========== Rewind ==========


	TUniquePtr<TRingBuffer<FGruxSnapshot>> RingBuffer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grux|Stun")
	bool bStunable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewind")
	int32 MaxRingBufferSize = 60 * 12 * 2;
	

	FGruxSnapshot LastValidSnapshot;

	void CaptureSnapshot() override;
	bool RewindOneFrame() override;
	void OnRewindStart_Implementation(bool bIsPassive);
	void OnRewindEnd_Implementation();

	UFUNCTION()
	UAnimMontage* FindMontageByName(FName MontageName);

	bool bIsRewinding = false;

	// ========== 状態 ==========
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grux|State")
	EEnemyState CurrentState = EEnemyState::Idle;

};
