#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/Image.h"
#include "DebugFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class THEGREENGROCERHUT_API UDebugFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, Category = "Debug", meta = (Keywords = "set brush texture safe valid", DefaultToSelf = "WidgetContext", HidePin = "WidgetContext"))
	static void SafeSetBrushFromTexture(UUserWidget* WidgetContext, UImage* TargetImage, UTexture2D* Texture);
};
