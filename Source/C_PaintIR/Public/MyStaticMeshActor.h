// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CanvasComponent.h"
#include "Engine/StaticMeshActor.h"
#include "MyStaticMeshActor.generated.h"

UENUM(BlueprintType)
enum class EViewDirection : uint8
{
	Front UMETA(DisplayName = "Front"),
	Back UMETA(DisplayName = "Back"),
	Right UMETA(DisplayName = "Right"),
	Left UMETA(DisplayName = "Left"),
	Top UMETA(DisplayName = "Top"),
	Bottom UMETA(DisplayName = "Bottom")
};

USTRUCT(BlueprintType)
struct FCameraViewInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FVector Location;

	UPROPERTY(BlueprintReadOnly)
	FRotator Rotation;

	FCameraViewInfo() {}
	FCameraViewInfo(const FVector& InLocation, const FRotator& InRotation)
		: Location(InLocation), Rotation(InRotation) {}
};
/**
 * 
 */
UCLASS()
class C_PAINTIR_API AMyStaticMeshActor : public AStaticMeshActor
{
	GENERATED_BODY()
	
public:
	AMyStaticMeshActor();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCanvasComponent* CanvasComponent;

	/** 获取指定方向的摄像机观察位置与朝向 */
	UFUNCTION(BlueprintCallable, Category = "Camera View")
	FCameraViewInfo GetViewPosition(EViewDirection Direction) const;

	/** 获取模型名称 */
	UFUNCTION(BlueprintCallable, Category = "Mesh")
	FString GetMeshName() const;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnActorClicked(AActor* TouchedActor, FKey ButtonPressed);
};
