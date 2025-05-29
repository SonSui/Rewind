// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "IRewindable.generated.h"

UINTERFACE(MinimalAPI)
class URewindable : public UInterface { GENERATED_BODY() };

class IRewindable
{
	GENERATED_BODY()

public:
	// スナップショット
	virtual void CaptureSnapshot() PURE_VIRTUAL(IRewindable::CaptureSnapshot, );

	// 1フレーム分巻き戻す
	virtual bool RewindOneFrame() PURE_VIRTUAL(IRewindable::RewindOneFrame, return false;);

	// 巻き戻し開始
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Rewind")
	void OnRewindStart(bool bIsPassive);

	// 巻き戻し終了
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Rewind")
	void OnRewindEnd();
};