// Fill out your copyright notice in the Description page of Project Settings.


#include "MyNewActor.h"

// Sets default values
AMyNewActor::AMyNewActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyNewActor::BeginPlay()
{
	Super::BeginPlay();
	

	
	CenterPoint = GetActorLocation();
	
}

// Called every frame
void AMyNewActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector lo = GetActorLocation();

	UE_LOG(LogTemp, Warning, TEXT("Tick"));

}

void AMyNewActor::MoveInCircle(float DeltaTime)
{
	// 現在の角度を更新
	CurrentAngle += AngularSpeed * DeltaTime;
	// 新しい位置を計算（XY平面で円運動）
	float X = CenterPoint.X + Radius * FMath::Cos(CurrentAngle);
	float Y = CenterPoint.Y + Radius * FMath::Sin(CurrentAngle);
	float Z = GetActorLocation().Z; // 高さ（Z）は維持
	// Actor の位置を更新
	SetActorLocation(FVector(X, Y, Z));
	UE_LOG(LogTemp, Warning, TEXT("Location: (%.2f, %.2f, %.2f)"), X, Y, Z);
	UE_LOG(LogTemp, Warning, TEXT("MoveInCircle !"));
}
