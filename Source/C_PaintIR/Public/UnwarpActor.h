// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnwarpActor.generated.h"

class USceneCaptureComponent2D;
class UStaticMeshComponent;

UCLASS()
class C_PAINTIR_API AUnwarpActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUnwarpActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	USceneCaptureComponent2D* SceneCaptureComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;
};
