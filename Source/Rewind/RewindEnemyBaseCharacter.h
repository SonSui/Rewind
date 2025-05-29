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
	Idle,        // ⋗�
	Patrolling,
	Chasing,
	Attacking,
	Stunned,
	Dead,
	Rewinding
};
/**
 * �G�L�����N�^�[�̊��N���X
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


	// �v���C���[�����F�������ɌĂ΂��iAI���m����j
	UFUNCTION(BlueprintCallable, Category = "AI")
	virtual void OnPlayerSpotted(APawn* PlayerPawn);

	// �_���[�W���󂯂��Ƃ��̏����i�e����I�[�o�[���C�h�j
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	// �U������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bIsAttacking = false;

	// �U���Ԋu�i�b�j
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackCooldown = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DefaultAttackCooldown = 2.0f;

	// �U���^�C�}�[
	FTimerHandle AttackCooldownTimer;

	// ========== �s���֐��iBTTask�EState�Ǘ��p�j ==========
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


	// ========== �⏕�֐� ==========
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
	// �s�����W�b�N�̈ꎞ��~ 
	UFUNCTION(BlueprintCallable, Category = "BT Control")
	virtual void StopBTLogic(const FString& Reason) {};

	// �s�����W�b�N�̍ĊJ 
	UFUNCTION(BlueprintCallable, Category = "BT Control")
	virtual void RestartBTLogic() {};
	UFUNCTION(BlueprintCallable, Category = "BT Control")
	virtual FVector GetSpawnPoint() const { return SpawnPoint; }

	virtual void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted) {};

	// ========== �s���t���O�iBTTask�EState�Ǘ��p�j ==========
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
	bool bCanPatrol = true;  // �p�g���[���\���ǂ���

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
	bool bOverPatrolRange = false;  // �p�g���[���͈͊O���ǂ���

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
	FVector MoveTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
	AActor* TargetActor;  // �U���Ώ�

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
	bool bPlayerDetected = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
	bool bPlayerInAttackRange = false;
	// ========== �p�g���[�� ==========


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