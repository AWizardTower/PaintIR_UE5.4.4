// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"
#include "MyStaticMeshActor.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraActor.h"

AMyPlayerController::AMyPlayerController()
{
	bEnableClickEvents = true;        // 启用点击事件
	bShowMouseCursor = true;          // 显示鼠标光标
	DefaultClickTraceChannel = ECC_Visibility; // 设置点击检测通道
	DefaultMouseCursor = EMouseCursor::Crosshairs;

	static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClassFinder(TEXT("/Game/UI/UMG_Main"));
	if (WidgetClassFinder.Succeeded())
	{
		UMGMainWidgetClass = WidgetClassFinder.Class;
	}
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UMGMainWidgetClass)
	{
		UUserWidget* UMGMainWidget = CreateWidget<UUserWidget>(this, UMGMainWidgetClass);
		if (UMGMainWidget)
		{
			UMGMainWidget->AddToViewport();
		}
	}
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
				MyStaticMeshActor->CanvasComponent->ApplyUnwrapMaterial(MyStaticMeshActor->GetStaticMeshComponent());
				// 或其他 CanvasComponent 的方法
			}
		}
	}
}

void AMyPlayerController::SetCurrentValue(float NewValue)
{
	CurrentValue = NewValue;
	// 可以在此处添加其他逻辑，例如更新 UI 显示等
}

void AMyPlayerController::MoveCharacter(const FVector& TargetLocation, const FRotator& TargetRotation)
{
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn)
	{
		ControlledPawn->SetActorLocation(TargetLocation);

		// 设置控制器的旋转，这样角色也会朝向你设定的方向
		SetControlRotation(TargetRotation);
	}
}