// Fill out your copyright notice in the Description page of Project Settings.


#include "MyStaticMeshActor.h"

AMyStaticMeshActor::AMyStaticMeshActor()
{
	CanvasComponent = CreateDefaultSubobject<UCanvasComponent>(TEXT("CanvasComponent"));
	CanvasComponent ->SetupAttachment(GetRootComponent());
	
}

void AMyStaticMeshActor::BeginPlay()
{
	Super::BeginPlay();

	// 绑定点击事件
	OnClicked.AddDynamic(this, &AMyStaticMeshActor::OnActorClicked);
}

void AMyStaticMeshActor::OnActorClicked(AActor* TouchedActor, FKey ButtonPressed)
{
	// if (TouchedActor)
	// {
	// 	// 获取点击位置
	// 	FVector HitLocation;
	// 	FVector HitNormal;
	// 	if (GetHitResultUnderCursor(ECC_Visibility, true, HitLocation, HitNormal))
	// 	{
	// 		// 将点击位置转换为字符串
	// 		FString HitLocationStr = HitLocation.ToString();
	//
	// 		// 在屏幕上显示点击位置
	// 		if (GEngine)
	// 		{
	// 			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Clicked at location: %s"), *HitLocationStr));
	// 		}
	// 	}
	// }
}

FCameraViewInfo AMyStaticMeshActor::GetViewPosition(EViewDirection Direction) const
{
	if (!GetStaticMeshComponent() || !GetStaticMeshComponent()->GetStaticMesh())
	{
		return FCameraViewInfo();
	}

	FBoxSphereBounds Bounds = GetStaticMeshComponent()->Bounds;
	FVector Center = Bounds.Origin;
	FVector Extent = Bounds.BoxExtent;
	float Distance = Extent.Size() * 1.5f;

	FVector ViewDirection;

	switch (Direction)
	{
	case EViewDirection::Front:
		ViewDirection = FVector(1, 0, 0);
		break;
	case EViewDirection::Back:
		ViewDirection = FVector(-1, 0, 0);
		break;
	case EViewDirection::Right:
		ViewDirection = FVector(0, 1, 0);
		break;
	case EViewDirection::Left:
		ViewDirection = FVector(0, -1, 0);
		break;
	case EViewDirection::Top:
		ViewDirection = FVector(0, 0, 1);
		break;
	case EViewDirection::Bottom:
		ViewDirection = FVector(0, 0, -1);
		break;
	default:
		ViewDirection = FVector(1, 0, 0);
		break;
	}

	FVector ViewLocation = Center + ViewDirection * Distance;
	FRotator ViewRotation = (Center - ViewLocation).Rotation();

	return FCameraViewInfo(ViewLocation, ViewRotation);
}

FString AMyStaticMeshActor::GetMeshName() const
{
	if (UStaticMeshComponent* MeshComp = GetStaticMeshComponent())
	{
		if (UStaticMesh* Mesh = MeshComp->GetStaticMesh())
		{
			return Mesh->GetName();
		}
	}
	return TEXT("None");
}