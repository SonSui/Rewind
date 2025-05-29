// Fill out your copyright notice in the Description page of Project Settings.

#include "RewindPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "RewindAnimInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/CharacterMovementComponent.h"

ARewindPlayerCharacter::ARewindPlayerCharacter()
{
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	PrimaryActorTick.bCanEverTick = true;

	// カメラアームの作成
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // カメラ距離
	CameraBoom->bUsePawnControlRotation = true; // カメラをコントローラーに追従

	// フォローカメラの作成
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // カメラは独自に回転しない

	PostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("RewindPP"));
	PostProcessComp->SetupAttachment(RootComponent);
	PostProcessComp->bUnbound = true;

	PrimaryActorTick.bCanEverTick = true;

	CurrentHealth = MaxHealth; // 初期体力設定

	RingBuffer = MakeUnique<TRingBuffer<FPlayerSnapshot>>(MaxRingBufferSize);
	bIsRewinding = false;
	bIsStunned = false;
	bWantsToCombo = false;
	bCanCheckCombo = false;
	ComboCount = 0;
	RewindableTime = MaxRewindableTime;
	LastValidSnapshot = FPlayerSnapshot();
	currState = EPlayerState::WalkingOrIdle;
}

void ARewindPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 入力マッピングを登録
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
	if (DesaturateMaterialBase) //モノクロ
	{
		DesaturateMID = UMaterialInstanceDynamic::Create(DesaturateMaterialBase, this);
		PostProcessComp->Settings.AddBlendable(DesaturateMID, 0.0f);
	}
}

void ARewindPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARewindPlayerCharacter::Move);
		}
		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARewindPlayerCharacter::Look);
		}
		if (AttackAction)
		{
			EnhancedInput->BindAction(AttackAction, ETriggerEvent::Started, this, &ARewindPlayerCharacter::Attack);
		}
		if (DefendAction)
		{
			EnhancedInput->BindAction(DefendAction, ETriggerEvent::Triggered, this, &ARewindPlayerCharacter::StartDefend);
			EnhancedInput->BindAction(DefendAction, ETriggerEvent::Completed, this, &ARewindPlayerCharacter::EndDefend);
		}
		if (RewindAction)
		{
			EnhancedInput->BindAction(RewindAction, ETriggerEvent::Started, this, &ARewindPlayerCharacter::OnRewindStartInput);
			EnhancedInput->BindAction(RewindAction, ETriggerEvent::Completed, this, &ARewindPlayerCharacter::OnRewindEnd_Implementation);
		}
	}
}
void ARewindPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if ((bIsDesaturating || bIsResaturating) && DesaturateMID)//　モノクロ
	{
		DesatElapsedTime += DeltaTime;
		float Alpha = FMath::Clamp(DesatElapsedTime / DesatFadeTime, 0.0f, 1.0f);
		float DesatValue = bIsDesaturating ? Alpha : (1.0f - Alpha);

		DesaturateMID->SetScalarParameterValue(FName("DesaturationParam"), DesatValue);
		PostProcessComp->Settings.WeightedBlendables.Array[0].Weight = DesatValue;
		if (Alpha >= 1.0f)
		{
			bIsDesaturating = bIsResaturating = false;
		}
	}


	// キャラクターの移動速度を計算
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

		if (URewindAnimInstance* AnimInstance = Cast<URewindAnimInstance>(GetMesh()->GetAnimInstance()))
		{
			AnimInstance->Speed = GetVelocity().Size();
			AnimInstance->moveAnimeSpeed = GetVelocity().Size() / GetCharacterMovement()->MaxWalkSpeed;
		}
		CaptureSnapshot();
	}

}

void ARewindPlayerCharacter::Move(const FInputActionValue& Value)
{
	if (
		currState == EPlayerState::Stuned ||
		currState == EPlayerState::Rewinding ||
		currState == EPlayerState::Dead ||
		currState == EPlayerState::Attacking ||
		currState == EPlayerState::Defending ||
		bIsRewinding
		)return;// 移動禁止
	const FVector2D MovementVector = Value.Get<FVector2D>();
	CurrentInputVector = MovementVector;

	const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : FRotator::ZeroRotator;
	const FRotator YawRotation(0, ControlRotation.Yaw, 0);

	// 世界方向へ変換
	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// 移動
	AddMovementInput(ForwardDir, MovementVector.Y);
	AddMovementInput(RightDir, MovementVector.X);

	// カメラにあわせて回転
	if (!MovementVector.IsNearlyZero())
	{
		const FVector WorldInput = ForwardDir * MovementVector.Y + RightDir * MovementVector.X;
		const FRotator TargetRotation = WorldInput.Rotation();
		
		const FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), 10.0f);
		SetActorRotation(NewRotation);
	}

	const FVector InputDirectionWorld = ForwardDir * MovementVector.Y + RightDir * MovementVector.X;

	// キャラクターのローカル座標系に変換
	const FVector InputInLocal = GetActorTransform().InverseTransformVector(InputDirectionWorld);

	// アニメーション用InputVectorに設定
	if (URewindAnimInstance* AnimInstance = Cast<URewindAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->InputVector = FVector2D(InputInLocal.Y, InputInLocal.X);
		
	}
	currState = EPlayerState::WalkingOrIdle;
}
void ARewindPlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
void ARewindPlayerCharacter::Attack()
{
	if (
		currState == EPlayerState::Stuned ||
		currState == EPlayerState::Rewinding ||
		currState == EPlayerState::Dead ||
		bIsRewinding
		)return;// 攻撃禁止
	

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (!AnimInstance || !AttackMontage) return;

	// 攻撃していない
	if (!AnimInstance->Montage_IsPlaying(AttackMontage))
	{
		bWantsToCombo = false;
		bCanCheckCombo = false;
		currState = EPlayerState::Attacking;
		ComboCount = 1;

		
		AnimInstance->Montage_Play(AttackMontage);

		FOnMontageBlendingOutStarted BlendOutDelegate;
		BlendOutDelegate.BindUObject(this, &ARewindPlayerCharacter::OnAttackMontageEnded);
		AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, AttackMontage);
		AnimInstance->Montage_JumpToSection(FName("AttackA_Begin"), AttackMontage);
	}
	else if (bCanCheckCombo)
	{
		// コンボ判定
		bWantsToCombo = true;
		
	}
}
void ARewindPlayerCharacter::PlayComboSection()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance || !AttackMontage) return;

	FName CurrentSection = AnimInstance->Montage_GetCurrentSection();

	if (ComboCount==1||CurrentSection == "AttackA_Connect"||CurrentSection=="AttackA_Over")
	{
		AnimInstance->Montage_JumpToSection(FName("AttackB_Begin"), AttackMontage);
		ComboCount = 2;
	}
	else if (ComboCount == 2 || CurrentSection == "AttackB_Connect" || CurrentSection == "AttackB_Over")
	{
		AnimInstance->Montage_JumpToSection(FName("AttackC_Begin"), AttackMontage);
		ComboCount = 0;
	}
}
void ARewindPlayerCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == AttackMontage)
	{
		if(currState!=EPlayerState::Stuned)currState = EPlayerState::WalkingOrIdle;
		bCanCheckCombo = false;
		bWantsToCombo = false;
		ComboCount = 0;
	}
	
}
void ARewindPlayerCharacter::CheckEnemiesByOverlap(float Radius, float HalfAngleDegrees, float MaxHeightDiff)
{
	TArray<AActor*> OverlappedActors;

	// 検索対象の敵クラス
	TArray<TSubclassOf<AActor>> ActorFilter;
	ActorFilter.Add(ARewindEnemyBaseCharacter::StaticClass());

	const FVector Center = GetActorLocation();
	const FVector Forward = GetActorForwardVector();

	// 球体内のアクターを取得
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Center,
		Radius,
		TArray<TEnumAsByte<EObjectTypeQuery>>{ EObjectTypeQuery::ObjectTypeQuery3 }, // Pawn等
		ARewindEnemyBaseCharacter::StaticClass(), // 敵キャラの親クラス
		TArray<AActor*>(), // 除外対象
		OverlappedActors
	);
	TSet<AActor*> DamagedActors;
	for (AActor* Actor : OverlappedActors)
	{
		if (!Actor || Actor == this) continue;
		if (DamagedActors.Contains(Actor))
			continue; 
		DamagedActors.Add(Actor);
		// 高低差チェック
		if (FMath::Abs(Actor->GetActorLocation().Z - Center.Z) > MaxHeightDiff)
			continue;

		FVector ToTarget = Actor->GetActorLocation() - Center;
		ToTarget.Normalize();

		float Dot = FVector::DotProduct(Forward, ToTarget);
		float Angle = FMath::RadiansToDegrees(FMath::Acos(Dot));

		if (Angle <= HalfAngleDegrees)
		{
			UE_LOG(LogTemp, Warning, TEXT("Player hit: %s"), *Actor->GetName());
			UGameplayStatics::ApplyDamage(Actor, 5.0f, GetController(), this, nullptr);
			FVector VictimLocation = Actor->GetActorLocation();
			UCapsuleComponent* VictimCapsule = Cast<UCapsuleComponent>(Actor->GetComponentByClass(UCapsuleComponent::StaticClass()));
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
			
			FVector DirToVictim = (VictimLocation - AttackerLocation).GetSafeNormal();
			
			DirToVictim.Z += 0.2f;
			DirToVictim.Normalize();

			// ノックバックの強さ
			float ImpulseStrength = 300.0f;

			// キャラクターの移動コンポーネントを取得
			UCharacterMovementComponent* MoveComp = Cast<UCharacterMovementComponent>(Actor->FindComponentByClass<UCharacterMovementComponent>());
			if (MoveComp && MoveComp->IsMovingOnGround())
			{
				// 衝撃を加える
				MoveComp->AddImpulse(DirToVictim * ImpulseStrength, true);
			}
			ARewindEnemyBaseCharacter* Enemy = Cast<ARewindEnemyBaseCharacter>(Actor);
			PlayAttackSound();
			ShowEnemyHealthBar(Enemy);
		}
	}
	
	// オーバーラップ範囲をデバッグ表示
	//DrawDebugSphere(GetWorld(), Center, Radius, 16, FColor::Red, false, 1.0f);
}
void ARewindPlayerCharacter::StartDefend()
{
	if (bIsRewinding||
		currState == EPlayerState::Rewinding ||
		currState == EPlayerState::Stuned ||
		currState == EPlayerState::Dead ||
		currState == EPlayerState::Defending 
		) return; // 防御禁止

	if (!DefendMontage) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	// 攻撃中なら強制終了
	if (currState == EPlayerState::Attacking)
	{
		AnimInstance->Montage_Stop(0.2f, AttackMontage);
		bWantsToCombo = false;
		bCanCheckCombo = false;
		ComboCount = 0;
	}

	currState = EPlayerState::Defending;

	AnimInstance->Montage_Play(DefendMontage);

}
void ARewindPlayerCharacter::EndDefend()
{
	if (currState == EPlayerState::Defending)
	{
		currState = EPlayerState::WalkingOrIdle;
		GetMesh()->GetAnimInstance()->Montage_Stop(0.2f, DefendMontage);
	}
}

void ARewindPlayerCharacter::CaptureSnapshot()
{
	FPlayerSnapshot Snapshot;

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
	if (URewindAnimInstance* AnimInstance = Cast<URewindAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Snapshot.InputVector = AnimInstance->InputVector;
		Snapshot.MoveAnimeSpeed = AnimInstance->moveAnimeSpeed;
		Snapshot.Speed = AnimInstance->Speed;
	}

	Snapshot.State = currState;
	Snapshot.bWantsToCombo = bWantsToCombo;
	Snapshot.bCanCheckCombo = bCanCheckCombo;
	Snapshot.ComboCount = ComboCount;
	Snapshot.TimeStamp = GetWorld()->TimeSeconds;

	RingBuffer->Push(Snapshot);
}
bool ARewindPlayerCharacter::RewindOneFrame()
{
	FPlayerSnapshot Snapshot;
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
	if (URewindAnimInstance* AnimInstance = Cast<URewindAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->InputVector = Snapshot.InputVector;
		AnimInstance->moveAnimeSpeed = -Snapshot.MoveAnimeSpeed; 
		AnimInstance->Speed = -Snapshot.Speed; // 巻き戻し中は逆再生
	}
	// 状態復元
	currState = Snapshot.State;
	bWantsToCombo = Snapshot.bWantsToCombo;
	bCanCheckCombo = Snapshot.bCanCheckCombo;
	ComboCount = Snapshot.ComboCount;
	RewindableTime -= GetWorld()->DeltaTimeSeconds;

	LastValidSnapshot = Snapshot;
	if (RewindableTime <= 0.0f)
	{
		OnRewindEnd_Implementation();
	}
	return true;
	
	
}
void ARewindPlayerCharacter::OnRewindStart_Implementation(bool bIsPassive)
{
	if (bIsRewinding) return; // 二重呼び出し防止
	StartDesaturationFade(true);
	bIsRewinding = true;
	currState = EPlayerState::Rewinding;

	// 他の全オブジェクトに巻き戻し開始を通知
	TArray<AActor*> Rewindables;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), URewindable::StaticClass(), Rewindables);

	for (AActor* Actor : Rewindables)
	{
		if (Actor && Actor->GetClass()->ImplementsInterface(URewindable::StaticClass()))
		{
			IRewindable::Execute_OnRewindStart(Actor, false);
		}
	}
	
	// Movement を停止
	GetCharacterMovement()->DisableMovement();

	// 状態ログ
	UE_LOG(LogTemp, Warning, TEXT("RewindStart"), bIsPassive ? TEXT("true") : TEXT("false"));
}
void ARewindPlayerCharacter::OnRewindEnd_Implementation()
{
	if (!bIsRewinding) return;
	if (bIsForceRewind) return;

	bIsRewinding = false;
	StartDesaturationFade(false);
	
	// 他の全オブジェクトに巻き戻し終了を通知
	TArray<AActor*> Rewindables;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), URewindable::StaticClass(), Rewindables);

	for (AActor* Actor : Rewindables)
	{
		if (Actor && Actor->GetClass()->ImplementsInterface(URewindable::StaticClass()))
		{
			IRewindable::Execute_OnRewindEnd(Actor);
		}
	}


	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		// Montage復元
		UAnimMontage* Montage = FindMontageByName(LastValidSnapshot.MontageName); 
		if (Montage)
		{
			AnimInstance->Montage_Play(Montage, 1.0f);
			AnimInstance->Montage_SetPosition(Montage, LastValidSnapshot.MontagePosition);
			currState = LastValidSnapshot.State;
			if (currState == EPlayerState::Attacking)
			{
				// 攻撃中なら攻撃終了処理追加
				FOnMontageBlendingOutStarted BlendOutDelegate;
				BlendOutDelegate.BindUObject(this, &ARewindPlayerCharacter::OnAttackMontageEnded);
				AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, AttackMontage);
			}
			if (currState == EPlayerState::Defending)EndDefend();
		}
		else
		{
			AnimInstance->StopAllMontages(0.0f); // 即停止
			currState = EPlayerState::WalkingOrIdle;
		}
	}

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	UE_LOG(LogTemp, Warning, TEXT("RewindEnd"));
}
UAnimMontage* ARewindPlayerCharacter::FindMontageByName(FName MontageName)
{
	if (AttackMontage && AttackMontage->GetFName() == MontageName)
		return AttackMontage;
	if (HitMontage && HitMontage->GetFName() == MontageName)
		return HitMontage;
	if (DeathMontage && DeathMontage->GetFName() == MontageName)
		return DeathMontage;
	if (DefendMontage && DefendMontage->GetFName() == MontageName)
		return DefendMontage;
	
	return nullptr;
	
}
void ARewindPlayerCharacter::OnRewindStartInput()
{
	if (bIsRewinding|| currState == EPlayerState::Stuned||currState == EPlayerState::Dead) return;
	if (RewindableTime <= 0.0f) return;
	OnRewindStart_Implementation(true);
}

float ARewindPlayerCharacter::TakeDamage(float DamageAmount,FDamageEvent const& DamageEvent,AController* EventInstigator,AActor* DamageCauser)
{
	if (bIsForceRewind||currState == EPlayerState::Dead) return 0;
	if (currState == EPlayerState::Defending) return 0;
	//float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	
	bIsForceRewind = false;

	
	if (HitMontage)
	{
		if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
		{
			if (!AnimInst->Montage_IsPlaying(HitMontage))
			{
				currState = EPlayerState::Stuned;

				AnimInst->Montage_Play(HitMontage);

			}
		}
	}

	return 1;
}
void ARewindPlayerCharacter::OnForceRewindNotify()
{

	if (RewindableTime <= 0.0f) {
		currState = EPlayerState::Dead;
		return;
	}
	if (bIsRewinding) return;
	bIsForceRewind = true;

	OnRewindStart_Implementation(false);


	GetWorld()->GetTimerManager().SetTimer(
		ForceRewindTimerHandle,
		this, &ARewindPlayerCharacter::EndForceRewind,
		3.0f, false
	);
}
void ARewindPlayerCharacter::EndForceRewind()
{
	bIsForceRewind = false;
	OnRewindEnd_Implementation();
	GetWorld()->GetTimerManager().ClearTimer(ForceRewindTimerHandle);
}
void ARewindPlayerCharacter::StartDesaturationFade(bool bToGray)
{
	if (!DesaturateMID) return;

	bIsDesaturating = bToGray;      // モノクロ
	bIsResaturating = !bToGray;     // 回復
	DesatElapsedTime = 0.0f;
}
void ARewindPlayerCharacter::ShowEnemyHealthBar(ARewindEnemyBaseCharacter* enemy)
{
	if (!enemy) return;
	if (CurrentEnemy)
	{
		if (!IsValid(CurrentEnemy) || CurrentEnemy->IsDead())
		{
			CurrentEnemy = nullptr;
		}
		else
		{
			return;
		}
	}

	CurrentEnemy = enemy;

	if (EnemyHealthBarWidget)
	{
		EnemyHealthBarWidget->RemoveFromParent(); // 前のUIを削除
		EnemyHealthBarWidget = nullptr;
	}

	// EnemyHealthBarを生成
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), EnemyHealthBarClass);
	if (!Widget) return;

	UFunction* GetEnemyFunc = Widget->FindFunction(FName("GetEnemy"));
	if (GetEnemyFunc)
	{
		Widget->ProcessEvent(GetEnemyFunc,nullptr); 
	}

	// 画面に表示
	Widget->AddToViewport();
	EnemyHealthBarWidget = Widget;

}