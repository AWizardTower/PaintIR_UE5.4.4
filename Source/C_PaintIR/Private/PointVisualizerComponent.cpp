// Fill out your copyright notice in the Description page of Project Settings.


#include "PointVisualizerComponent.h"

#include "KeyPointUserWidget.h"
#include "Components/WidgetComponent.h"

// Sets default values for this component's properties
UPointVisualizerComponent::UPointVisualizerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ConstructorHelpers 只能在构造函数里用
	 static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClassFinder(TEXT("/Game/UI/UMG_Point.UMG_Point"));
	 if (WidgetClassFinder.Succeeded())
	 {
	 	KeyPointWidgetClass = WidgetClassFinder.Class;
	 }
	// ...
}


// Called when the game starts
void UPointVisualizerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UPointVisualizerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPointVisualizerComponent::AddKeyPoint(const FVector& WorldPosition, float Value)
{
	AActor* Owner = GetOwner();
	if (!Owner || !SphereMesh || !KeyPointWidgetClass) return;
	
	// 创建球体组件
	UStaticMeshComponent* Sphere = NewObject<UStaticMeshComponent>(Owner);
	Sphere->SetStaticMesh(SphereMesh);
	Sphere->SetMaterial(0, SphereMaterial);
	Sphere->SetWorldLocation(WorldPosition);
	Sphere->RegisterComponent();
	Sphere->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	
	// 创建 WidgetComponent
	UWidgetComponent* Widget = NewObject<UWidgetComponent>(Owner);
	Widget->SetWidgetClass(KeyPointWidgetClass);
	Widget->SetDrawSize(FVector2D(150, 50));
	Widget->SetWorldLocation(WorldPosition + FVector(0, 0, 30)); // 比球体略高一点
	Widget->SetWidgetSpace(EWidgetSpace::World);
	Widget->RegisterComponent();
	Widget->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);

	if (UUserWidget* CreatedWidget = Widget->GetUserWidgetObject())
	{
		if (UKeyPointUserWidget* KeyPointWidget = Cast<UKeyPointUserWidget>(CreatedWidget))
		{
			//// 初始化：设置标签、值等
			//KeyPointWidget->SetLabel(FText::FromString(FString::Printf(TEXT("%.2f"), Value)));
			//KeyPointWidget->UpdateValue(Value);
			//KeyPointWidget->BindRemoveButton(); // 由蓝图绑定委托
		}
	}
	
	// 存储结构体
	FKeyPointVisual Visual;
	Visual.Position = WorldPosition;
	Visual.Value = Value;
	Visual.SphereComponent = Sphere;
	Visual.WidgetComponent = Widget;
	
	KeyPointVisuals.Add(Visual);
}

// 修改一个关键点的值
void UPointVisualizerComponent::ModifyKeyPoint(const FVector& Position, float NewValue)
{
	for (FKeyPointVisual& Point : KeyPointVisuals)
	{
		if (Point.Position == Position)
		{
			Point.Value = NewValue;
			break;
		}
	}
}

// 删除一个关键点
void UPointVisualizerComponent::RemoveKeyPoint(const FVector& Position)
{
	KeyPointVisuals.RemoveAll([&](const FKeyPointVisual& Point) {
		return Point.Position == Position;
	});
}
