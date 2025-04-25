// Fill out your copyright notice in the Description page of Project Settings.


#include "KeyPointUserWidget.h"

void UKeyPointUserWidget::ModifyValue(float NewValue) {

}

void UKeyPointUserWidget::RemovePoint() {

}

// KeyPointUserWidget.cpp
void UKeyPointUserWidget::CommitValue(float NewValue)
{
	OnValueCommitted.Broadcast(NewValue);
}