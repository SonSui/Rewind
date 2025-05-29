// Fill out your copyright notice in the Description page of Project Settings.


#include "BTDecorator_CheckBlackboardBool.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_CheckBlackboardBool::UBTDecorator_CheckBlackboardBool()
{
	NodeName = TEXT("Check Blackboard Bool == True");
}

bool UBTDecorator_CheckBlackboardBool::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	// BlackboardからBool値を取得
    const UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        //UE_LOG(LogTemp, Error, TEXT("Blackboard component not found"));
        return false;
    }

    const bool bValue = BlackboardComp->GetValueAsBool(BoolKey.SelectedKeyName);

    // ブラックボードキーとその値をログ出力する
    //UE_LOG(LogTemp, Warning, TEXT("UBTDecorator_CheckBlackboardBool: %s = %s"),*BoolKey.SelectedKeyName.ToString(),bValue ? TEXT("true") : TEXT("false"));

    return bValue;
}