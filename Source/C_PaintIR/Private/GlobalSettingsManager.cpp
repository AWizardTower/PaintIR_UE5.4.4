// Fill out your copyright notice in the Description page of Project Settings.


#include "GlobalSettingsManager.h"

TArray<FString> UGlobalSettingsManager::GetEnumOptionsForComboBox() const
{
	TArray<FString> DisplayList;

	// 获取 ETextureSize 枚举的 UEnum 对象
	UEnum* Enum = StaticEnum<ETextureSize>();
	if (!Enum) return DisplayList;

	// 获取枚举值的数量
	const int32 EnumCount = Enum->NumEnums() - 1;  // -1 是因为有一个 "Max" 类型，通常不作为实际值使用

	// 遍历枚举并添加显示名称
	for (int32 i = 0; i <= EnumCount; ++i)
	{
		// 获取枚举值的显示名称
		FString DisplayName = Enum->GetDisplayNameTextByValue(i).ToString();
        
		// 添加到显示名称列表
		DisplayList.Add(DisplayName);
	}

	return DisplayList;
}