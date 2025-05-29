// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyGruxCharacter.h"
#include "RewindPlayerCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BrainComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationPath.h" 
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Character.h"
#include "RewindEnemyController.h"
#include "Engine/DamageEvents.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemyGruxAnimInstance.h"
#include "Kismet/GameplayStatics.h"

AEnemyGruxCharacter::AEnemyGruxCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	RingBuffer = MakeUnique<TRingBuffer<FGruxSnapshot>>(MaxRingBufferSize);
}

void AEnemyGruxCharacter::BeginPlay()
{
	Super::BeginPlay();

	// スポーン地点を保存
	SpawnPoint = GetActorLocation();

	// 初期状態
	CurrentState = EEnemyState::Idle;

	// 初期値リセット
	AttackCooldown = DefaultAttackCooldown;
	StunTime = 0.0f;
	Toughness = DefaultToughness;

	// 最大HPなど
	CurrentHealth = MaxHealth;
	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController && BehaviorTreeAsset)
	{
		AIController->RunBehaviorTree(BehaviorTreeAsset);  // ビヘイビアツリーを実行する
		UE_LOG(LogTemp, Warning, TEXT("Grux: Behavior Tree Started"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Grux: Failed to start Behavior Tree"));
	}
}

void AEnemyGruxCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bIsRewinding)
	{
		const bool bSuccess = RewindOneFrame();
		if (!bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("RingBuffer is empty"));
			OnRewindEnd_Implementation();
		}
	}
	else
	{
		// 攻撃クールダウン減少
		if (AttackCooldown > 0.f)
		{
			AttackCooldown -= DeltaSeconds;
		}
		if (UEnemyGruxAnimInstance* AnimInstance = Cast<UEnemyGruxAnimInstance>(GetMesh()->GetAnimInstance()))
		{
			AnimInstance->Speed = GetVelocity().Size();
			AnimInstance->moveAnimeSpeed = GetVelocity().Size() / GetCharacterMovement()->MaxWalkSpeed;
		}	
		CaptureSnapshot();
	}
	
}
float AEnemyGruxCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// 既に死亡なら何もしない
	if (CurrentState == EEnemyState::Dead )
	{
		return 0.f;
	}

	if (CurrentHealth <= 0.0f)
	{
		OnRewindEnd_Implementation();
		EnterDeath();
	}
	else if (CurrentState == EEnemyState::Attacking&&bStunable)
	{
		// 攻撃中なら韧性を減らす
		Toughness = FMath::Max(0, Toughness - 1);

		if (Toughness <= 0)
		{
			EnterStun();
		}
	}
	else if (CurrentState == EEnemyState::Patrolling)
	{
		StopBTLogic(TEXT("Patrol Interrupted"));
		TryTurnToPlayer();
		RestartBTLogic();
	}
	TryTurnToPlayer();
	return ActualDamage;
}

bool AEnemyGruxCharacter::EnterIdle(bool bShouldRestartBT)
{
	if (CurrentState == EEnemyState::Dead) return false; 
	CurrentState = EEnemyState::Idle;
	bIsAttacking = false;
	bStunable = false;

	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		AI->StopMovement();
	}

	bPlayerDetected = false;
	bPlayerInAttackRange = false;

	if (bShouldRestartBT)
	{
		RestartBTLogic();
	}
	return true;
}
void AEnemyGruxCharacter::EnterStun()
{
	// 状態をスタンに変更
	CurrentState = EEnemyState::Stunned;
	bStunable = false;
	// 行動を一時停止（BTを停止）
	StopBTLogic(TEXT("Stunned"));

	// スタン時間を初期化
	StunTime = DefaultStunTime;
	Toughness = DefaultToughness;
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		AI->StopMovement();
	}

	// アニメーションを再生（Stun用Montageがある場合）
	if (StunMontage)
	{
		UAnimInstance* Anim = GetMesh()->GetAnimInstance();
		if (Anim && !Anim->Montage_IsPlaying(StunMontage))
		{
			Anim->Montage_Play(StunMontage);
		}
	}

}
void AEnemyGruxCharacter::ResetStunTime()
{
	StunTime = 0.0f;
}
void AEnemyGruxCharacter::ResetToughness()
{
	Toughness = DefaultToughness;
}
bool AEnemyGruxCharacter::EnterAttack()
{
	if (!CanAttack())
	{
		//EnterIdle();
		return false;
	}
	

	CurrentState = EEnemyState::Attacking;
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		AI->StopMovement();
	}
	AttackCooldown = DefaultAttackCooldown; // クールダウンリセット
	bIsAttacking = true;
	const bool bUseAttack2 = FMath::RandRange(0, 100) < 40; // 40%の確率で攻撃2
	if (bUseAttack2)
	{
		StartAttack2();
	}
	else
	{
		StartAttack1();
	}
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		if (AI->BrainComponent)
		{
			AI->BrainComponent->StopLogic(TEXT("Attacking"));
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Grux: Attack Started"));
	return true;
}
void AEnemyGruxCharacter::StartAttack1()
{
	if (!AttackPreMontage || !AttackMontage) return;

	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (!Anim) return;

	// 前モーション再生
	Anim->Montage_Play(AttackPreMontage);

	bStunable = true;

	// 前モーション終了後に本体攻撃再生
	FOnMontageEnded EndDelegate;
	EndDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted) {
		if (!bInterrupted && AttackMontage)
		{
			GetMesh()->GetAnimInstance()->Montage_Play(AttackMontage);
		}
		else
		{
			EnterIdle(false); // 中断時は即Idle
		}
		});

	GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate, AttackPreMontage);
	
}
void AEnemyGruxCharacter::StartAttack2()
{
	if (!AttackMontage2) return;

	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (!Anim) return;

	
	bStunable = true;             // この間だけスタン可能

	Anim->Montage_Play(AttackMontage2);
	FOnMontageEnded EndDelegate;
	EndDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted) {
			EnterIdle(false); 
		});

	GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate, AttackMontage2);
	
}
bool AEnemyGruxCharacter::CanAttack() const
{
	// 死亡・硬直・回溯中は攻撃不可
	if (CurrentState == EEnemyState::Dead ||
		CurrentState == EEnemyState::Stunned ||
		CurrentState == EEnemyState::Rewinding)
	{
		return false;
	}

	// 攻撃間隔中も攻撃不可
	if (AttackCooldown > 0.f)
	{
		return false;
	}

	return true;
}
bool AEnemyGruxCharacter::IsInAttackRange(const FVector& PlayerLocation) const
{
	// 座標を比較（Z無視）
	FVector MyLocation = GetActorLocation();
	FVector ToPlayer = PlayerLocation - MyLocation;
	ToPlayer.Z = 0.0f;

	// 距離チェック
	const float Distance = ToPlayer.Size();
	if (Distance > AttackRange)
	{
		return false;
	}
	else return true;
}
bool AEnemyGruxCharacter::IsPlayerVisible() const
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn) return false;

	FVector MyLocation = GetActorLocation();
	FVector PlayerLocation = PlayerPawn->GetActorLocation();

	// 高さ無視して水平距離で計算
	FVector ToPlayer = PlayerLocation - MyLocation;
	ToPlayer.Z = 0.0f;

	// 距離チェック
	const float Distance = ToPlayer.Size();
	if (Distance > SightRadius)
	{
		return false;
	}
	if (CurrentState != EEnemyState::Attacking && CurrentState != EEnemyState::Chasing)
	{// 角度チェック
		ToPlayer.Normalize();
		FVector Forward = GetActorForwardVector().GetSafeNormal2D();
		const float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Forward, ToPlayer)));
		if (Angle > SightAngle)
		{
			return false;
		}
	}

	// 視線遮蔽チェック
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		MyLocation + FVector(0, 0, 50),
		PlayerLocation + FVector(0, 0, 50),
		ECollisionChannel::ECC_Visibility,
		Params
	);

	// 遮蔽物に当たった場合はプレイヤーが見えない
	if (bHit && Hit.GetActor() != PlayerPawn)
	{
		return false;
	}

	return true;
}
bool AEnemyGruxCharacter::MoveToLocation(const FVector& Location)
{
	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI) return false;

	FAIMoveRequest Request;
	Request.SetGoalLocation(Location);
	Request.SetAcceptanceRadius(50.f); // 近くに到達すれば成功扱い

	FNavPathSharedPtr OutPath;
	AI->MoveTo(Request, &OutPath);
	return true;
}
bool AEnemyGruxCharacter::EnterChase(AActor* PlayerActor)
{
	
	if (!PlayerActor) return false;

	// 状態をChasingに設定
	CurrentState = EEnemyState::Chasing;

	// プレイヤーの位置を取得
	const FVector PlayerLocation = PlayerActor->GetActorLocation();
	const FVector MyLocation = GetActorLocation();

	// プレイヤーに向かう方向（水平）
	FVector ToPlayer = (PlayerLocation - MyLocation);
	ToPlayer.Z = 0.0f;
	ToPlayer.Normalize();

	// 攻撃範囲の2/3で止まる
	MoveTargetLocation = PlayerLocation - ToPlayer * (AttackRange * 0.33f);
	
	return true;
}
bool AEnemyGruxCharacter::EnterPatrol()
{
	CurrentState = EEnemyState::Patrolling;

	UWorld* World = GetWorld();
	if (!World) return false;

	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI) return false;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	if (!NavSys) return false;

	FNavLocation ProjectedPoint;
	const FVector Origin = SpawnPoint;
	bool bFound = false;

	// 最大20回試行
	for (int i = 0; i < 20; ++i)
	{
		const FVector RandomDir = FMath::VRand().GetSafeNormal2D();
		const FVector Candidate = Origin + RandomDir * PatrolRadius;

		if (NavSys->ProjectPointToNavigation(Candidate, ProjectedPoint, FVector(100, 100, 100)))
		{
			// 可達な地点を発見
			MoveTargetLocation = ProjectedPoint.Location;
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		// Spawn地点へ戻る
		MoveTargetLocation = SpawnPoint;
	}

	return true;
}
void AEnemyGruxCharacter::EnterDeath()
{
	// すでに死亡状態なら何もしない
	if (CurrentState == EEnemyState::Dead) return;

	// 状態をDeadに変更
	CurrentState = EEnemyState::Dead;

	// 行動停止
	StopBTLogic(TEXT("Dead"));

	// 移動を停止
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		AI->StopMovement();
	}

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->StopMovementImmediately();  
		MoveComp->DisableMovement();          
		MoveComp->GravityScale = 0.0f;        
	}

	// アニメーションを再生（死亡用モンタージュ）
	if (DeathMontage)
	{
		UAnimInstance* Anim = GetMesh()->GetAnimInstance();
		if (Anim && !Anim->Montage_IsPlaying(DeathMontage))
		{
			Anim->Montage_Play(DeathMontage);
		}
	}


	// 一定時間後に削除（任意）
	FTimerHandle DeathTimer;
	GetWorldTimerManager().SetTimer(DeathTimer, [this]()
		{
			Destroy();
		}, 5.0f, false); // 5秒後削除

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetActorEnableCollision(false);
}
void AEnemyGruxCharacter::StopBTLogic(const FString& Reason)
{
	if (ARewindEnemyController* AI = Cast<ARewindEnemyController>(GetController()))
	{
		if (AI->BrainComponent && AI->BrainComponent->IsRunning())
		{
			AI->BrainComponent->StopLogic(Reason);
		}
	}
}
void AEnemyGruxCharacter::RestartBTLogic()
{
	if (ARewindEnemyController* AI = Cast<ARewindEnemyController>(GetController()))
	{
		if (AI->BrainComponent)
		{
			AI->BrainComponent->RestartLogic();
		}
	}
	bStunable = false;
}
void AEnemyGruxCharacter::FaceTarget(const FVector& TargetLocation)
{
	FVector ToTarget = TargetLocation - GetActorLocation();
	ToTarget.Z = 0.0f;

	FRotator NewRotation = ToTarget.Rotation();
	SetActorRotation(NewRotation);
}
void AEnemyGruxCharacter::TryTurnToPlayer()
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Player) return;

	FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.0f;
	ToPlayer.Normalize();

	FVector Forward = GetActorForwardVector().GetSafeNormal2D();
	float Dot = FVector::DotProduct(Forward, ToPlayer);
	float Angle = FMath::RadiansToDegrees(FMath::Acos(Dot));

	if (Angle > 30.0f) 
	{
		if (AAIController* AI = Cast<AAIController>(GetController()))
		{
			if (UBlackboardComponent* BB = AI->GetBlackboardComponent())
			{
				BB->SetValueAsBool(TEXT("bShouldTurnToPlayer"), true);
				BB->SetValueAsVector(TEXT("TargetLocation"), Player->GetActorLocation());
				BB->SetValueAsObject(TEXT("TargetActor"), Player);
				BB->SetValueAsBool(TEXT("bPlayerDetected"), true);
			}
		}
	}
}


void AEnemyGruxCharacter::CaptureSnapshot()
{
	FGruxSnapshot Snapshot;

	// 現在位置と回転を記録
	Snapshot.Location = GetActorLocation();
	Snapshot.Rotation = GetActorRotation();

	// 再生中Montageを取得
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
		if (CurrentMontage)
		{
			Snapshot.MontageName = CurrentMontage->GetFName();
			Snapshot.MontagePosition = AnimInstance->Montage_GetPosition(CurrentMontage);
		}
	}
	if (UEnemyGruxAnimInstance* AnimInstance = Cast<UEnemyGruxAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Snapshot.MoveAnimeSpeed = AnimInstance->moveAnimeSpeed;
		Snapshot.Speed = AnimInstance->Speed;
	}

	Snapshot.State = CurrentState;
	Snapshot.AttackCooldown = AttackCooldown;
	Snapshot.bStunable = bStunable;
	Snapshot.Toughness = Toughness;
	Snapshot.TimeStamp = GetWorld()->TimeSeconds;

	RingBuffer->Push(Snapshot);
}
bool AEnemyGruxCharacter::RewindOneFrame()
{
	FGruxSnapshot Snapshot;
	if (!RingBuffer->Pop(Snapshot))
	{
		return false;
	}

	// 位置と回転を復元
	SetActorLocation(Snapshot.Location);
	SetActorRotation(Snapshot.Rotation);

	// モンタージュ復元
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{

		UAnimMontage* Montage = FindMontageByName(Snapshot.MontageName);
		if (!Montage)
		{
			AnimInstance->StopAllMontages(0.0f); // 即停止
		}
		else
		{
			if (!AnimInstance->Montage_IsPlaying(Montage))
			{
				AnimInstance->Montage_Play(Montage, 1.0f);
			}
			AnimInstance->Montage_SetPosition(Montage, Snapshot.MontagePosition);
		}


	}
	if (UEnemyGruxAnimInstance* AnimInstance = Cast<UEnemyGruxAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->moveAnimeSpeed = -Snapshot.MoveAnimeSpeed;
		AnimInstance->Speed = -Snapshot.Speed; // 巻き戻し中は逆再生
	}
	// 状態復元
	CurrentState = Snapshot.State;

	AttackCooldown = Snapshot.AttackCooldown;
	bStunable = Snapshot.bStunable;
	Toughness = Snapshot.Toughness;


	LastValidSnapshot = Snapshot;
	
	return true;


}
void AEnemyGruxCharacter::OnRewindStart_Implementation(bool bIsPassive)
{
	if (bIsRewinding) return; // 二重呼び出し防止
	if (CurrentState == EEnemyState::Dead)return;
	bIsRewinding = true;
	CurrentState = EEnemyState::Rewinding;
	StopBTLogic(TEXT("Rewind"));
	// Movement を停止
	GetCharacterMovement()->DisableMovement();
	
}
void AEnemyGruxCharacter::OnRewindEnd_Implementation()
{
	if (!bIsRewinding) return;

	bIsRewinding = false;


	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		// Montage復元
		UAnimMontage* Montage = FindMontageByName(LastValidSnapshot.MontageName);
		if (Montage)
		{
			AnimInstance->Montage_Play(Montage, 1.0f);
			AnimInstance->Montage_SetPosition(Montage, LastValidSnapshot.MontagePosition);
			CurrentState = LastValidSnapshot.State;
			if (CurrentState == EEnemyState::Attacking||CurrentState == EEnemyState::Stunned)
			{
				// Montage終わるまでビヘイビアーツリーを停止
				FOnMontageBlendingOutStarted BlendOutDelegate;
				BlendOutDelegate.BindUObject(this, &AEnemyGruxCharacter::OnMontageBlendingOut);
				AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, Montage);;
			}
			
		}
		else
		{
			AnimInstance->StopAllMontages(0.0f); // 即停止
			CurrentState = EEnemyState::Idle;
			RestartBTLogic();
		}
	}

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	UE_LOG(LogTemp, Warning, TEXT("RewindEnd"));
}
UAnimMontage* AEnemyGruxCharacter::FindMontageByName(FName MontageName)
{
	if (AttackMontage && AttackMontage->GetFName() == MontageName)
		return AttackMontage;
	if (HitMontage && HitMontage->GetFName() == MontageName)
		return HitMontage;
	if (DeathMontage && DeathMontage->GetFName() == MontageName)
		return DeathMontage;
	if (AttackPreMontage && AttackPreMontage->GetFName() == MontageName)
		return AttackPreMontage;
	if (AttackMontage2 && AttackMontage2->GetFName() == MontageName)
		return AttackMontage2;
	if (StunMontage && StunMontage->GetFName() == MontageName)
		return StunMontage;

	return nullptr;

}
void AEnemyGruxCharacter::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	// BT を再起動
	if (CurrentState == EEnemyState::Attacking || CurrentState == EEnemyState::Stunned)
	{
		RestartBTLogic();
	}
}
void AEnemyGruxCharacter::CheckPlayerByOverlap(float Radius, float HalfAngleDegrees, float MaxHeightDiff)
{
	if (bIsRewinding) return;
	TArray<AActor*> OverlappedActors;

	// プレイヤークラスだけを対象にする
	TArray<TSubclassOf<AActor>> ActorFilter;
	ActorFilter.Add(ARewindPlayerCharacter::StaticClass());

	const FVector Center = GetActorLocation();
	const FVector Forward = GetActorForwardVector();

	// オーバーラップ検索
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Center,
		Radius,
		TArray<TEnumAsByte<EObjectTypeQuery>>{ EObjectTypeQuery::ObjectTypeQuery3 },
		nullptr, 
		TArray<AActor*>(),
		OverlappedActors
	);

	for (AActor* Actor : OverlappedActors)
	{
		if (!Actor || Actor == this) continue;

		// プレイヤー以外は無視
		ARewindPlayerCharacter* Player = Cast<ARewindPlayerCharacter>(Actor);
		if (!Player) continue;

		// 高低差チェック
		if (FMath::Abs(Actor->GetActorLocation().Z - Center.Z) > MaxHeightDiff)
			continue;

		FVector ToTarget = Actor->GetActorLocation() - Center;
		ToTarget.Normalize();

		float Dot = FVector::DotProduct(Forward, ToTarget);
		float Angle = FMath::RadiansToDegrees(FMath::Acos(Dot));

		if (Angle <= HalfAngleDegrees)
		{
			// ヒットログ出力
			UE_LOG(LogTemp, Warning, TEXT("プレイヤー命中: %s"), *Actor->GetName());

			// ダメージ適用
			Actor->TakeDamage(0.0f, FDamageEvent(), GetController(), this);

			// ヒットエフェクト
			FVector VictimLocation = Actor->GetActorLocation();
			FVector AttackerLocation = GetActorLocation();
			FVector DirToAttacker = (AttackerLocation - VictimLocation).GetSafeNormal();
			FVector SpawnLocation = VictimLocation + DirToAttacker;
			FRotator SpawnRotation = DirToAttacker.Rotation();

			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				HitSparkEffect,
				SpawnLocation,
				SpawnRotation,
				FVector(1.0f),
				true,
				true,
				ENCPoolMethod::None,
				true
			);

			// ノックバック
			FVector DirToVictim = (VictimLocation - AttackerLocation).GetSafeNormal();
			DirToVictim.Z += 0.2f;
			DirToVictim.Normalize();
			float ImpulseStrength = 300.0f;

			UCharacterMovementComponent* MoveComp = Cast<UCharacterMovementComponent>(Actor->FindComponentByClass<UCharacterMovementComponent>());
			if (MoveComp && MoveComp->IsMovingOnGround())
			{
				MoveComp->AddImpulse(DirToVictim * ImpulseStrength, true);
				PlayAttackSound();
			}
		}
	}
}