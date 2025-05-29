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

	// �J�����A�[���̍쐬
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // �J��������
	CameraBoom->bUsePawnControlRotation = true; // �J�������R���g���[���[�ɒǏ]

	// �t�H���[�J�����̍쐬
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // �J�����͓Ǝ��ɉ�]���Ȃ�

	PostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("RewindPP"));
	PostProcessComp->SetupAttachment(RootComponent);
	PostProcessComp->bUnbound = true;

	PrimaryActorTick.bCanEverTick = true;

	CurrentHealth = MaxHealth; // �����̗͐ݒ�

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

	// ���̓}�b�s���O��o�^
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
	if (DesaturateMaterialBase) //���m�N��
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

	if ((bIsDesaturating || bIsResaturating) && DesaturateMID)//�@���m�N��
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


	// �L�����N�^�[�̈ړ����x���v�Z
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
		)return;// �ړ��֎~
	const FVector2D MovementVector = Value.Get<FVector2D>();
	CurrentInputVector = MovementVector;

	const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : FRotator::ZeroRotator;
	const FRotator YawRotation(0, ControlRotation.Yaw, 0);

	// ���E�����֕ϊ�
	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// �ړ�
	AddMovementInput(ForwardDir, MovementVector.Y);
	AddMovementInput(RightDir, MovementVector.X);

	// �J�����ɂ��킹�ĉ�]
	if (!MovementVector.IsNearlyZero())
	{
		const FVector WorldInput = ForwardDir * MovementVector.Y + RightDir * MovementVector.X;
		const FRotator TargetRotation = WorldInput.Rotation();
		
		const FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), 10.0f);
		SetActorRotation(NewRotation);
	}

	const FVector InputDirectionWorld = ForwardDir * MovementVector.Y + RightDir * MovementVector.X;

	// �L�����N�^�[�̃��[�J�����W�n�ɕϊ�
	const FVector InputInLocal = GetActorTransform().InverseTransformVector(InputDirectionWorld);

	// �A�j���[�V�����pInputVector�ɐݒ�
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
		)return;// �U���֎~
	

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (!AnimInstance || !AttackMontage) return;

	// �U�����Ă��Ȃ�
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
		// �R���{����
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

	// �����Ώۂ̓G�N���X
	TArray<TSubclassOf<AActor>> ActorFilter;
	ActorFilter.Add(ARewindEnemyBaseCharacter::StaticClass());

	const FVector Center = GetActorLocation();
	const FVector Forward = GetActorForwardVector();

	// ���̓��̃A�N�^�[���擾
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Center,
		Radius,
		TArray<TEnumAsByte<EObjectTypeQuery>>{ EObjectTypeQuery::ObjectTypeQuery3 }, // Pawn��
		ARewindEnemyBaseCharacter::StaticClass(), // �G�L�����̐e�N���X
		TArray<AActor*>(), // ���O�Ώ�
		OverlappedActors
	);
	TSet<AActor*> DamagedActors;
	for (AActor* Actor : OverlappedActors)
	{
		if (!Actor || Actor == this) continue;
		if (DamagedActors.Contains(Actor))
			continue; 
		DamagedActors.Add(Actor);
		// ���፷�`�F�b�N
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

			// �m�b�N�o�b�N�̋���
			float ImpulseStrength = 300.0f;

			// �L�����N�^�[�̈ړ��R���|�[�l���g���擾
			UCharacterMovementComponent* MoveComp = Cast<UCharacterMovementComponent>(Actor->FindComponentByClass<UCharacterMovementComponent>());
			if (MoveComp && MoveComp->IsMovingOnGround())
			{
				// �Ռ���������
				MoveComp->AddImpulse(DirToVictim * ImpulseStrength, true);
			}
			ARewindEnemyBaseCharacter* Enemy = Cast<ARewindEnemyBaseCharacter>(Actor);
			PlayAttackSound();
			ShowEnemyHealthBar(Enemy);
		}
	}
	
	// �I�[�o�[���b�v�͈͂��f�o�b�O�\��
	//DrawDebugSphere(GetWorld(), Center, Radius, 16, FColor::Red, false, 1.0f);
}
void ARewindPlayerCharacter::StartDefend()
{
	if (bIsRewinding||
		currState == EPlayerState::Rewinding ||
		currState == EPlayerState::Stuned ||
		currState == EPlayerState::Dead ||
		currState == EPlayerState::Defending 
		) return; // �h��֎~

	if (!DefendMontage) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	// �U�����Ȃ狭���I��
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

	// ���݈ʒu�Ɖ�]���L�^
	Snapshot.Location = GetActorLocation();
	Snapshot.Rotation = GetActorRotation();

	// �Đ���Montage���擾
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

	// �ʒu�Ɖ�]�𕜌�
	SetActorLocation(Snapshot.Location);
	SetActorRotation(Snapshot.Rotation);

	// �����^�[�W������
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		
		UAnimMontage* Montage = FindMontageByName(Snapshot.MontageName);
		if (!Montage)
		{
			AnimInstance->StopAllMontages(0.0f); // ����~
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
		AnimInstance->Speed = -Snapshot.Speed; // �����߂����͋t�Đ�
	}
	// ��ԕ���
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
	if (bIsRewinding) return; // ��d�Ăяo���h�~
	StartDesaturationFade(true);
	bIsRewinding = true;
	currState = EPlayerState::Rewinding;

	// ���̑S�I�u�W�F�N�g�Ɋ����߂��J�n��ʒm
	TArray<AActor*> Rewindables;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), URewindable::StaticClass(), Rewindables);

	for (AActor* Actor : Rewindables)
	{
		if (Actor && Actor->GetClass()->ImplementsInterface(URewindable::StaticClass()))
		{
			IRewindable::Execute_OnRewindStart(Actor, false);
		}
	}
	
	// Movement ���~
	GetCharacterMovement()->DisableMovement();

	// ��ԃ��O
	UE_LOG(LogTemp, Warning, TEXT("RewindStart"), bIsPassive ? TEXT("true") : TEXT("false"));
}
void ARewindPlayerCharacter::OnRewindEnd_Implementation()
{
	if (!bIsRewinding) return;
	if (bIsForceRewind) return;

	bIsRewinding = false;
	StartDesaturationFade(false);
	
	// ���̑S�I�u�W�F�N�g�Ɋ����߂��I����ʒm
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
		// Montage����
		UAnimMontage* Montage = FindMontageByName(LastValidSnapshot.MontageName); 
		if (Montage)
		{
			AnimInstance->Montage_Play(Montage, 1.0f);
			AnimInstance->Montage_SetPosition(Montage, LastValidSnapshot.MontagePosition);
			currState = LastValidSnapshot.State;
			if (currState == EPlayerState::Attacking)
			{
				// �U�����Ȃ�U���I�������ǉ�
				FOnMontageBlendingOutStarted BlendOutDelegate;
				BlendOutDelegate.BindUObject(this, &ARewindPlayerCharacter::OnAttackMontageEnded);
				AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, AttackMontage);
			}
			if (currState == EPlayerState::Defending)EndDefend();
		}
		else
		{
			AnimInstance->StopAllMontages(0.0f); // ����~
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

	bIsDesaturating = bToGray;      // ���m�N��
	bIsResaturating = !bToGray;     // ��
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
		EnemyHealthBarWidget->RemoveFromParent(); // �O��UI���폜
		EnemyHealthBarWidget = nullptr;
	}

	// EnemyHealthBar�𐶐�
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), EnemyHealthBarClass);
	if (!Widget) return;

	UFunction* GetEnemyFunc = Widget->FindFunction(FName("GetEnemy"));
	if (GetEnemyFunc)
	{
		Widget->ProcessEvent(GetEnemyFunc,nullptr); 
	}

	// ��ʂɕ\��
	Widget->AddToViewport();
	EnemyHealthBarWidget = Widget;

}