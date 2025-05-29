// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "IRewindable.generated.h"

UINTERFACE(MinimalAPI)
class URewindable : public UInterface { GENERATED_BODY() };

class IRewindable
{
	GENERATED_BODY()

public:
	// �X�i�b�v�V���b�g
	virtual void CaptureSnapshot() PURE_VIRTUAL(IRewindable::CaptureSnapshot, );

	// 1�t���[���������߂�
	virtual bool RewindOneFrame() PURE_VIRTUAL(IRewindable::RewindOneFrame, return false;);

	// �����߂��J�n
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Rewind")
	void OnRewindStart(bool bIsPassive);

	// �����߂��I��
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Rewind")
	void OnRewindEnd();
};