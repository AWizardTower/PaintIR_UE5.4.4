// Fill out your copyright notice in the Description page of Project Settings.


#include "MyStaticMeshActor.h"

AMyStaticMeshActor::AMyStaticMeshActor()
{
	CanvasComponent = CreateDefaultSubobject<UCanvasComponent>(TEXT("CanvasComponent"));
	CanvasComponent ->SetupAttachment(GetRootComponent());
	
}

void AMyStaticMeshActor::BeginPlay()
{
	Super::BeginPlay();

	// 绑定点击事件
	OnClicked.AddDynamic(this, &AMyStaticMeshActor::OnActorClicked);
}

void AMyStaticMeshActor::OnActorClicked(AActor* TouchedActor, FKey ButtonPressed)
{
	// if (TouchedActor)
	// {
	// 	// 获取点击位置
	// 	FVector HitLocation;
	// 	FVector HitNormal;
	// 	if (GetHitResultUnderCursor(ECC_Visibility, true, HitLocation, HitNormal))
	// 	{
	// 		// 将点击位置转换为字符串
	// 		FString HitLocationStr = HitLocation.ToString();
	//
	// 		// 在屏幕上显示点击位置
	// 		if (GEngine)
	// 		{
	// 			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Clicked at location: %s"), *HitLocationStr));
	// 		}
	// 	}
	// }
}