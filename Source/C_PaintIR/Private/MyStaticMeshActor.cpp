// Fill out your copyright notice in the Description page of Project Settings.


#include "MyStaticMeshActor.h"

AMyStaticMeshActor::AMyStaticMeshActor()
{
	CanvasComponent = CreateDefaultSubobject<UCanvasComponent>(TEXT("CanvasComponent"));
	CanvasComponent ->SetupAttachment(GetRootComponent());
}