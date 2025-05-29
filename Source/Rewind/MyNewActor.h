// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyNewActor.generated.h"

UCLASS()
class REWIND_API AMyNewActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyNewActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    // ��]�̒��S�_
    UPROPERTY(EditAnywhere, Category = "CircularMotion")
    FVector CenterPoint = FVector(0.0f, 0.0f, 0.0f);

    // ���a�i���j�b�g�P�ʁj
    UPROPERTY(EditAnywhere, Category = "CircularMotion")
    float Radius = 300.0f;

    // �p���x�i���W�A�����b�j
    UPROPERTY(EditAnywhere, Category = "CircularMotion")
    float AngularSpeed = 1.0f;

    UFUNCTION(BlueprintCallable)
    void MoveInCircle(float DeltaTime);

private:
    // ���݂̊p�x�i���W�A���j
    float CurrentAngle = 0.0f;

    

};
