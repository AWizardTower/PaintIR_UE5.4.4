// Fill out your copyright notice in the Description page of Project Settings.


#include "CanvasComponent.h"

// Sets default values for this component's properties
UCanvasComponent::UCanvasComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCanvasComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UCanvasComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCanvasComponent::DrawPoint(const FVector& WorldLocation)
{
	// 将世界坐标转换为组件的局部坐标
	FVector LocalLocation = GetComponentTransform().InverseTransformPosition(WorldLocation);
	// 输出局部坐标
	UE_LOG(LogTemp, Log, TEXT("Clicked at local position: %s"), *LocalLocation.ToString());
}

