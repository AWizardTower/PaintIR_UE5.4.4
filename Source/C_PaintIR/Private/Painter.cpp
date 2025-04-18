// Fill out your copyright notice in the Description page of Project Settings.


#include "Painter.h"

APainter::APainter()
{
	UStaticMeshComponent* MeshComp = GetMeshComponent(); // 获取默认的网格组件
	if (MeshComp)
	{
		MeshComp->SetVisibility(false);
	}
}