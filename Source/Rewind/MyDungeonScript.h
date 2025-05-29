// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "MyDungeonScript.generated.h"

/**
 * 
 */
UCLASS()
class REWIND_API AMyDungeonScript : public ALevelScriptActor
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	bool bIsPlayerDead = false;

	UFUNCTION(BlueprintCallable)
	void SetPlayerDeadFlag() { bIsPlayerDead = true; }
};
