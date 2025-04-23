// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PointComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class C_PAINTIR_API UPointComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPointComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(BlueprintReadWrite, Category = "Point")
    FVector Position;

    UPROPERTY(BlueprintReadWrite, Category = "Point")
    float Value;

    //// Íø¸ñÌå×é¼þ£¨ÓÃÓÚÏÔÊ¾¹Ø¼üµãÎ»ÖÃ£©
    //UPROPERTY(VisibleAnywhere, Category = "Point")
    //UStaticMeshComponent* MeshComponent;

    //// UMG Widget ×é¼þ£¨ÏÔÊ¾¹Ø¼üµãµÄÖµ£©
    //UPROPERTY(VisibleAnywhere, Category = "Point")
    //UWidgetComponent* WidgetComponent;

    //// ¸üÐÂ¹Ø¼üµã¿ÉÊÓ»¯£¨¸üÐÂÍø¸ñºÍUMG£©
    //void UpdateVisualizer();

    //// ÐÞ¸Ä¹Ø¼üµãÖµ
    //void ModifyValue(float NewValue);

    //// ÉèÖÃ¹Ø¼üµãµÄÎ»ÖÃ
    //void RemovePoint();
};
