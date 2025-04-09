// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CanvasComponent.h"
#include "Engine/StaticMeshActor.h"
#include "MyStaticMeshActor.generated.h"

/**
 * 
 */
UCLASS()
class C_PAINTIR_API AMyStaticMeshActor : public AStaticMeshActor
{
	GENERATED_BODY()
	
public:
	AMyStaticMeshActor();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCanvasComponent* CanvasComponent;
};
