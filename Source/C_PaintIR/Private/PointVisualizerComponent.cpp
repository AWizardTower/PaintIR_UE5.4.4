// Fill out your copyright notice in the Description page of Project Settings.


#include "PointVisualizerComponent.h"

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

// Called when the game starts
void UPointVisualizerComponent::AddKeyPoint(const FVector& WorldPosition, float Value)
{


}

