// Fill out your copyright notice in the Description page of Project Settings.


#include "PointVisualizerComponent.h"

#include "KeyPointUserWidget.h"
#include "PointComponent.h"
#include "Components/WidgetComponent.h"
#include "CanvasComponent.h"

// Sets default values for this component's properties
UPointVisualizerComponent::UPointVisualizerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
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

// void UPointVisualizerComponent::AddKeyPoint(const FVector& WorldPosition, float Value)
// {
// 	AActor* Owner = GetOwner();
// 	if (!Owner || !SphereMesh || !KeyPointWidgetClass) return;
// 	
// 	// 创建球体组件
// 	UStaticMeshComponent* Sphere = NewObject<UStaticMeshComponent>(Owner);
// 	Sphere->SetStaticMesh(SphereMesh);
// 	Sphere->SetMaterial(0, SphereMaterial);
// 	Sphere->SetWorldLocation(WorldPosition);
// 	Sphere->RegisterComponent();
// 	Sphere->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
// 	
// 	// 创建 WidgetComponent
// 	UWidgetComponent* Widget = NewObject<UWidgetComponent>(Owner);
// 	Widget->SetWidgetClass(KeyPointWidgetClass);
// 	Widget->SetDrawSize(FVector2D(150, 50));
// 	Widget->SetWorldLocation(WorldPosition + FVector(0, 0, 30)); // 比球体略高一点
// 	Widget->SetWidgetSpace(EWidgetSpace::World);
// 	Widget->RegisterComponent();
// 	Widget->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
//
// 	if (UUserWidget* CreatedWidget = Widget->GetUserWidgetObject())
// 	{
// 		if (UKeyPointUserWidget* KeyPointWidget = Cast<UKeyPointUserWidget>(CreatedWidget))
// 		{
// 			//// 初始化：设置标签、值等
// 			//KeyPointWidget->SetLabel(FText::FromString(FString::Printf(TEXT("%.2f"), Value)));
// 			//KeyPointWidget->UpdateValue(Value);
// 			//KeyPointWidget->BindRemoveButton(); // 由蓝图绑定委托
// 		}
// 	}
// 	
// 	// 存储结构体
// 	FKeyPointVisual Visual;
// 	Visual.Position = WorldPosition;
// 	Visual.Value = Value;
// 	Visual.SphereComponent = Sphere;
// 	Visual.WidgetComponent = Widget;
// 	
// 	KeyPointVisuals.Add(Visual);
// }

void UPointVisualizerComponent::AddKeyPoint(const FVector& Position, float Value)
{

	// // 创建一个球体静态网格组件
	// UStaticMeshComponent* SphereComponent = NewObject<UStaticMeshComponent>(GetOwner());
	//
	// if (SphereComponent)
	// {
	// 	SphereComponent->RegisterComponent();
	// 	SphereComponent->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	//
	// 	UStaticMesh* SphereMesh1 = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	// 	if (SphereMesh1 && SphereComponent)
	// 	{
	// 		SphereComponent->SetStaticMesh(SphereMesh1);
	// 	}
	//
	// 	// 设置位置
	// 	SphereComponent->SetRelativeLocation(Position);
	// }	
	// 创建一个新的关键点组件
	UPointComponent* NewPointComponent = NewObject<UPointComponent>(this);
	NewPointComponent->RegisterComponent();
	// 如果没有显式设置位置，它会默认位于 UPointVisualizerComponent 的原点位置（即 FVector(0, 0, 0) 相对于 UPointVisualizerComponent）。
	NewPointComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	NewPointComponent->SetRelativeLocation(Position);
	NewPointComponent->OwningVisualizer = this;
	
	// 先传值，再初始化移动和显示
	NewPointComponent->SetPosition(Position);  // 设置关键点的位置
	NewPointComponent->SetValue(Value);    // 设置关键点的值
	// NewPointComponent->UpdateVisualizer();

	// UE_LOG(LogTemp, Warning, TEXT("AddKeyPoint: setting Position = %s, Value = %f"), *Position.ToString(), Value);
	// 将该关键点组件加入管理列表
	PointComponents.Add(NewPointComponent);
}

// Canvas靠位置查找点 也可以靠编号
void UPointVisualizerComponent::ModifyKeyPointValue(const FVector& WorldPosition, float NewValue)
{
	OwningCanvas->ModifyPointValue(WorldPosition, NewValue);
	//应该是修改canvas中的数据
	// if (PointComponent)
	// {
	// 	PointComponent->ModifyValue(NewValue);  // 修改关键点值
	// }
}

void UPointVisualizerComponent::RemoveKeyPoint(UPointComponent* PointComponent)
{
	if (PointComponent)
	{
		PointComponents.Remove(PointComponent);
		PointComponent->Cleanup();
		PointComponent->DestroyComponent();  // 销毁组件
	}

	OwningCanvas->RemovePoint(PointComponent->Position);
}

void UPointVisualizerComponent::UpdateAllVisualizers(const TMap<FVector, float>& NewPoints)
{
	// 清除旧的可视化点组件
	for (UPointComponent* PointComp : PointComponents)
	{
		if (PointComp)
		{
			PointComp->Cleanup();        // 可选：自定义清理逻辑（比如解绑事件）
			PointComp->DestroyComponent();
		}
	}
	PointComponents.Empty();

	// 添加新的可视化点
	for (const TPair<FVector, float>& Pair : NewPoints)
	{
		AddKeyPoint(Pair.Key, Pair.Value);
	}
}