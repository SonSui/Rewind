// Fill out your copyright notice in the Description page of Project Settings.


#include "MyRoundMoveActor.h"

// Sets default values
AMyRoundMoveActor::AMyRoundMoveActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyRoundMoveActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyRoundMoveActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UE_LOG(LogTemp, Warning, TEXT("hello?"));
	FVector NewLocation = GetActorLocation();
}

