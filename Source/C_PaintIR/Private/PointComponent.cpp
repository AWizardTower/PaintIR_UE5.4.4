// Fill out your copyright notice in the Description page of Project Settings.


#include "PointComponent.h"
#include "KeyPointUserWidget.h"
#include "PointVisualizerComponent.h"

// Sets default values for this component's properties
UPointComponent::UPointComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// 创建网格体组件
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(this);  // 让网格体组件依附于当前组件
	MeshComponent->SetRelativeScale3D(FVector(0.1f));

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialAsset(TEXT("/Game/Materials/M_Point.M_Point"));
	if (MaterialAsset.Succeeded())
	{
		MeshComponent->SetMaterial(0, MaterialAsset.Object);
	}
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshAsset.Succeeded())
	{
		MeshComponent->SetStaticMesh(SphereMeshAsset.Object);
	}

	
	// ConstructorHelpers 只能在构造函数里用
	static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClassFinder(TEXT("/Game/UI/UMG_Point"));
	if (WidgetClassFinder.Succeeded())
	{
		KeyPointWidgetClass = WidgetClassFinder.Class;
	}
	// 创建 UMG 组件
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(this);  // 让 UMG 组件依附于当前组件
	WidgetComponent->SetRelativeLocation(FVector(0, 0, 0)); // 没用
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetInitialLayerZOrder(999);
	// 设置Widget绘制大小，比如设成宽300，高200
	// 只改变了绘制盒子的大小，视觉观感上好像改变了相对位置
	WidgetComponent->SetDrawSize(FVector2D(500.0f, 500.0f));

	if (KeyPointWidgetClass)
	{
		WidgetComponent->SetWidgetClass(KeyPointWidgetClass);
	}
}


// Called when the game starts
void UPointComponent::BeginPlay()
{
	Super::BeginPlay();

	// 为什么得手动注册一下啊？我不行了
	MeshComponent->RegisterComponent();
	// 啊啊啊我服了
	WidgetComponent->RegisterComponent();
	// GetUserWidgetObject() 返回的是 运行时实例，这个实例通常是在 BeginPlay() 或更晚阶段才会被创建。
	UKeyPointUserWidget* Widget = Cast<UKeyPointUserWidget>(WidgetComponent->GetUserWidgetObject());
	if (Widget)
	{
		 Widget->OwningPoint = this;
		//Widget->OnValueCommitted.AddDynamic(this, &UPointComponent::UpdateValue);
		// 初始化
		// 绑定回调
		// Widget->OnValueCommitted.AddDynamic(this, &UMyComponent::OnWidgetValueChanged);
	}
}	

void UPointComponent::UpdateVisualizer()
{
}

void UPointComponent::InitialUI()
{
	
	// 因为没有存，每次都获取并Cast
	if (WidgetComponent && WidgetComponent->GetUserWidgetObject())
	{
		UKeyPointUserWidget* UserWidget = Cast<UKeyPointUserWidget>(WidgetComponent->GetUserWidgetObject());
		if (UserWidget)
		{
			//UserWidget->SetValue(Value);  // 更新 UMG 上的关键点值
		}
	}
}

// Called every frame
void UPointComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPointComponent::SetValue(float NewValue)
{
	// 更新关键点值
	Value = NewValue;
	
}

void UPointComponent::UpdateValue(float NewValue)
{
	Value = NewValue;
	OwningVisualizer->ModifyKeyPointValue(this->Position, NewValue);
}

void UPointComponent::RemovePoint()
{
	OwningVisualizer->RemoveKeyPoint(this);
}

void UPointComponent::SetPosition(const FVector& NewPosition)
{
	Position = NewPosition;
	//UpdateVisualizer();  // 更新网格位置
}

void UPointComponent::Cleanup()
{
	if (MeshComponent)
	{
		MeshComponent->DestroyComponent();
		MeshComponent = nullptr;
	}

	if (WidgetComponent)
	{
		WidgetComponent->DestroyComponent();
		WidgetComponent = nullptr;
	}
}