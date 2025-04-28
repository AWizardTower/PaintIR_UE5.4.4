// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ComboBoxString.h"
#include "UObject/NoExportTypes.h"
#include "GlobalSettingsManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTextureSizeChanged);
/**
 * 定义纹理尺寸的枚举
*/
UENUM(BlueprintType)
enum class ETextureSize : uint8
{
	Size_256 UMETA(DisplayName = "256x256"),
	Size_512 UMETA(DisplayName = "512x512"),
	Size_1024 UMETA(DisplayName = "1024x1024"),
	Size_2048 UMETA(DisplayName = "2048x2048")
};

/**
 * 结构体用于保存最大和最小值
 */
USTRUCT(BlueprintType)
struct FValueRange
{
	GENERATED_BODY()

	// 最小值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 MinValue = 0;

	// 最大值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 MaxValue = 100;
};

/**
 * 
 */
//创建一个专门的参数结构 FCanvasRenderSettings，在多个类中作为成员使用或传递
UCLASS()
class C_PAINTIR_API UGlobalSettingsManager : public UObject
{
	GENERATED_BODY()
	
private:
	// 当前的TextureSize（使用枚举类型）
	ETextureSize TextureSize = ETextureSize::Size_2048;
	
	// 保存最大和最小值的结构体
	FValueRange TextureSizeRange;
public:
	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnTextureSizeChanged OnTexureSizeChanged;

	// 当设置变化时调用
	void NotifySettingsChanged()
	{
		OnTexureSizeChanged.Broadcast();
	}
	
	// 设置TextureSize
	UFUNCTION(BlueprintCallable, Category = "Canvas Settings")
	void SetTextureSize(ETextureSize NewSize)
	{
		TextureSize = NewSize;
		NotifySettingsChanged();
		if (GEngine)
		{
			FString DebugText = FString::Printf(TEXT("TextureSize Changed To: %d"), GetTextureSizeValue());
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, DebugText);
		}
	}

	// 获取当前的TextureSize
	UFUNCTION(BlueprintCallable, Category = "Canvas Settings")
	ETextureSize GetTextureSize() const
	{
		return TextureSize;
	}

	// 设置最大最小值范围
	UFUNCTION(BlueprintCallable, Category = "Canvas Settings")
	void SetTextureSizeRange(int32 Min, int32 Max)
	{
		TextureSizeRange.MinValue = Min;
		TextureSizeRange.MaxValue = Max;
	}
	
	// 获取最大最小值范围
	UFUNCTION(BlueprintCallable, Category = "Canvas Settings")
	FValueRange GetTextureSizeRange() const
	{
		return TextureSizeRange;
	}
	
	TArray<FString> GetEnumOptionsForComboBox() const; 

	int32 GetTextureSizeValue()
	{
		switch (TextureSize)
		{
		case ETextureSize::Size_256:
			return 256;
		case ETextureSize::Size_512:
			return 512;
		case ETextureSize::Size_1024:
			return 1024;
		case ETextureSize::Size_2048:
			return 2048;
		default:
			return 128;  // 默认值
		}
	}
};
