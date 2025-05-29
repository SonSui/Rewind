// Fill out your copyright notice in the Description page of Project Settings.


#include "RewindAnimInstance.h"
#include "MyDungeonScript.h"
#include "RewindPlayerCharacter.h"

void URewindAnimInstance::ComboWindowCheck()
{
	
	if (ARewindPlayerCharacter* PC = Cast<ARewindPlayerCharacter>(TryGetPawnOwner()))
	{
		PC->bCanCheckCombo = true;
	}
}

void URewindAnimInstance::ComboWindowEnd()
{
	if (ARewindPlayerCharacter* PC = Cast<ARewindPlayerCharacter>(TryGetPawnOwner()))
	{
		PC->bCanCheckCombo = false;

		if (PC->bWantsToCombo)
		{
			PC->PlayComboSection();
			PC->bWantsToCombo = false;
		}
	}
	
}
void URewindAnimInstance::DefenceStop()
{
	if (ARewindPlayerCharacter* PC = Cast<ARewindPlayerCharacter>(TryGetPawnOwner()))
	{
		if (PC->DefendMontage && Montage_IsPlaying(PC->DefendMontage))
		{
			Montage_Pause(PC->DefendMontage); // ñhå‰ÉÇÉìÉ^Å[ÉWÉÖÇàÍéûí‚é~
		}
	}
}
void URewindAnimInstance::DefenceContinue()
{
	if (ARewindPlayerCharacter* PC = Cast<ARewindPlayerCharacter>(TryGetPawnOwner()))
	{
		if (PC->DefendMontage)
		{
			Montage_Resume(PC->DefendMontage); // çƒäJ
		}
	}
}
void URewindAnimInstance::TriggerAttack()
{
	if (ARewindPlayerCharacter* PC = Cast<ARewindPlayerCharacter>(TryGetPawnOwner()))
	{
		PC->CheckEnemiesByOverlap(); // çUåÇîªíË
	}
}
void URewindAnimInstance::HitStartRewind()
{
	if (ARewindPlayerCharacter* PC = Cast<ARewindPlayerCharacter>(TryGetPawnOwner()))
	{
		PC->OnForceRewindNotify();
	}
}
void URewindAnimInstance::PlayerDead()
{
	if (ARewindPlayerCharacter* PC = Cast<ARewindPlayerCharacter>(TryGetPawnOwner()))
	{
		PC->GetMesh()->bPauseAnims = true;
		PC->currState = EPlayerState::Dead;
		AActor* Owner = TryGetPawnOwner();
		if (!Owner) return;

		UWorld* World = Owner->GetWorld();
		if (!World) return;

		if (AMyDungeonScript* MyScript = Cast<AMyDungeonScript>(World->GetLevelScriptActor()))
		{
			MyScript->bIsPlayerDead = true;
		}

		
	}
}