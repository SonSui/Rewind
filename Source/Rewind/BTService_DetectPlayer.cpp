// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_DetectPlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "RewindEnemyBaseCharacter.h"
#include "RewindPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UBTService_DetectPlayer::UBTService_DetectPlayer()
{
	NodeName = TEXT("Detect Player");
	Interval = 0.3f; // 0.3秒ごとにチェック
}

void UBTService_DetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	ARewindEnemyBaseCharacter* Grux = Cast<ARewindEnemyBaseCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Grux) return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;


	bool wasVisible = BB->GetValueAsBool(TEXT("bPlayerDetected"));
	bool bVisible = Grux->IsPlayerVisible();

	BB->SetValueAsBool(TEXT("bPlayerDetected"), bVisible);

	if (bVisible)
	{
		APawn* Player = UGameplayStatics::GetPlayerPawn(Grux->GetWorld(), 0);
		BB->SetValueAsObject(TEXT("TargetActor"), Player);

		if (ARewindPlayerCharacter* PC = Cast<ARewindPlayerCharacter>(Player))
		{
			PC->ShowEnemyHealthBar(Grux);
		}
	}
	else
	{
		BB->ClearValue(TEXT("TargetActor"));
	}


	if (bVisible != wasVisible)
	{
		if (AAIController* AI = Cast<AAIController>(Grux->GetController()))
		{
			if (AI->BrainComponent)
			{
				AI->BrainComponent->RestartLogic(); // 巡回中でも強制切り替え
			}
		}
	}

	
	//UE_LOG(LogTemp, Warning, TEXT("UBTService_DetectPlayer : %s"), bVisible ? TEXT("true") : TEXT("false"));
}