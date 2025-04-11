#pragma once

#include "UObject/Interface.h"
#include "DrawableInterface.generated.h"

UINTERFACE(MinimalAPI)
class UDrawableInterface : public UInterface
{
	GENERATED_BODY()
};

class C_PAINTIR_API IDrawableInterface
{
	GENERATED_BODY()

public:
	virtual void Draw(const FVector& WorldLocation, float Value) = 0;
};
