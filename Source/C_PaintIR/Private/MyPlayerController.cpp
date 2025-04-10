// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

AMyPlayerController::AMyPlayerController()
{
	bEnableClickEvents = true;        // 启用点击事件
	bShowMouseCursor = true;          // 显示鼠标光标
	DefaultClickTraceChannel = ECC_Visibility; // 设置点击检测通道
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("LeftClick", IE_Pressed, this, &AMyPlayerController::HandleLeftClick);
}


void AMyPlayerController::HandleLeftClick()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("被调用了！"));

	FHitResult HitResult;
	if (GetHitResultUnderCursor(ECC_Visibility, true, HitResult))
	{
		FVector HitLocation = HitResult.Location;

		// 将点击位置转换为字符串
		FString HitLocationStr = HitLocation.ToString();

		// 在屏幕上显示点击位置
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Clicked at location: %s"), *HitLocationStr));
		}
	}
}

