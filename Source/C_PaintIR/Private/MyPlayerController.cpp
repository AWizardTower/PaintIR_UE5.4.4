// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"
#include "MyStaticMeshActor.h"

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
	FHitResult HitResult;
	if (GetHitResultUnderCursor(ECC_Visibility, true, HitResult))
	{
		AActor* HitActor = HitResult.GetActor();
		FVector WorldHitLocation = HitResult.Location;
		if (HitActor)
		{
			// 检查被点击的 Actor 是否是 AMyStaticMeshActor 的实例
			AMyStaticMeshActor* MyStaticMeshActor = Cast<AMyStaticMeshActor>(HitActor);
			if (MyStaticMeshActor && MyStaticMeshActor->CanvasComponent)
			{
				// 调用 CanvasComponent 的功能，例如绘制内容
				MyStaticMeshActor->CanvasComponent->SetMeshMaterial(MyStaticMeshActor->GetStaticMeshComponent());
				// 或其他 CanvasComponent 的方法
			}
		}
	}
}

