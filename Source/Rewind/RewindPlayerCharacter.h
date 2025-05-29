// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RewindBaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "NiagaraSystem.h"
#include "RingBuffer.h"
#include "IRewindable.h"
#include "TimerManager.h"
#include "RewindEnemyBaseCharacter.h"
#include "RewindPlayerCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UPostProcessComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UENUM(BlueprintType)
enum class EPlayerState : uint8 {
    WalkingOrIdle   UMETA(DisplayName = "Walking"),    // �ړ���
    Attacking		UMETA(DisplayName = "Attacking"),  // �U����
    Defending		UMETA(DisplayName = "Defending"),  // �h�䒆
    Rewinding		UMETA(DisplayName = "Rewinding"),  // ��蒆
    Dead			UMETA(DisplayName = "Dead"),      // ���S
	Stuned			UMETA(DisplayName = "Stuned"),    // �C��
};


USTRUCT()
struct FPlayerSnapshot
{
	GENERATED_BODY()

	UPROPERTY() FVector_NetQuantize Location;
	UPROPERTY() FRotator Rotation;
	UPROPERTY() FName   MontageName;  
	UPROPERTY() float MontagePosition = 0.0f;
	UPROPERTY() FVector2D InputVector;
	UPROPERTY() float Speed = 0.0f;
	UPROPERTY() float MoveAnimeSpeed = 0.0f;
	

	UPROPERTY() EPlayerState State = EPlayerState::WalkingOrIdle;
	UPROPERTY() bool bWantsToCombo = false;
	UPROPERTY() bool bCanCheckCombo = false;
	UPROPERTY() int32 ComboCount = 0;
	float TimeStamp = 0.0f;
};
/**
 * �v���C���[�L�����N�^�[�N���X
 */
UCLASS()
class REWIND_API ARewindPlayerCharacter : public ARewindBaseCharacter, public IRewindable
{
	GENERATED_BODY()

public:
	ARewindPlayerCharacter();
	virtual float TakeDamage(float DamageAmount,struct FDamageEvent const& DamageEvent,class AController* EventInstigator,AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** �ړ����� */
	void Move(const FInputActionValue& Value);

	/** ���_���� */
	void Look(const FInputActionValue& Value);

	/** �U������ */
	void Attack();

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* HitSparkEffect;

	void Tick(float DeltaTime) override;

	FTimerHandle ForceRewindTimerHandle;
	
	UPROPERTY(VisibleAnywhere, Category = "PostProcess")
	UPostProcessComponent* PostProcessComp;

	
	UPROPERTY(EditAnywhere, Category = "PostProcess")
	UMaterialInterface* DesaturateMaterialBase;

	
	UPROPERTY()
	UMaterialInstanceDynamic* DesaturateMID;

	
	bool bIsDesaturating = false;     
	bool bIsResaturating = false;     
	float DesatFadeTime = 0.5f; 
	float DesatElapsedTime = 0.0f;    

	
	void StartDesaturationFade(bool bToGray);


protected:
	/** �J�����A�[�� */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	// �t�H���[�J����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	// �f�t�H���g���̓}�b�s���O
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	// �ړ��A�N�V����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	// ���_�A�N�V����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	// �U���A�N�V����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DefendAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RewindAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	FVector2D CurrentInputVector;
	



public:
	

	// �h��A�j���[�V����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* DefendMontage;


	void PlayComboSection();

	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CheckEnemiesByOverlap(float Radius = 250.0f, float HalfAngleDegrees = 60.0f, float MaxHeightDiff = 100.0f);

	// �h��A�N�V�����J�n 
	void StartDefend();

	// �h��A�N�V�����I��
	void EndDefend();


	void ShowEnemyHealthBar(ARewindEnemyBaseCharacter* enemy);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	ARewindEnemyBaseCharacter* CurrentEnemy = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	UUserWidget* EnemyHealthBarWidget = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> EnemyHealthBarClass;

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	ARewindEnemyBaseCharacter* GetCurrentEnemy() const { return CurrentEnemy; }


public:
	// ���Ԋ����߂�
	TUniquePtr<TRingBuffer<FPlayerSnapshot>> RingBuffer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewind")
	int32 MaxRingBufferSize = 60 * 12 * 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewind")
	float MaxRewindableTime = 12.0f; // �ő努���߂�����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewind")
	float RewindableTime = 12.0f; // �����߂�����

	FPlayerSnapshot LastValidSnapshot;

	void CaptureSnapshot() override;
	bool RewindOneFrame() override;
	void OnRewindStart_Implementation(bool bIsPassive);
	void OnRewindEnd_Implementation();

	UFUNCTION()
	UAnimMontage* FindMontageByName(FName MontageName);

	UFUNCTION()
	void OnRewindStartInput();

	UPROPERTY()
	bool bIsRewinding = false;

	UFUNCTION(BlueprintCallable, Category = "Hit")
	void OnForceRewindNotify();

	void EndForceRewind();

	
	// �U�����Ɏ��̃R���{
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation)
	bool bWantsToCombo = false;

	// �R���{���͎�t��Ԃ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation)
	bool bCanCheckCombo = false;

	// �R���{��
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation)
	int32 ComboCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewind")
	bool bIsForceRewind = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerState")
	EPlayerState currState = EPlayerState::WalkingOrIdle;
};