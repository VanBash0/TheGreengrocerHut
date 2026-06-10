#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SympromsStructs.h"
#include "SymptomFunctionLibary.generated.h"

UCLASS()
class THEGREENGROCERHUT_API USymptomFunctionLibary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    UFUNCTION(BlueprintCallable, Category = "Symptoms")
    static bool GetOverlayVisual(const TInstancedStruct<FVisualBase>& Visual, FVisualOverlay& OutOverlay);

    UFUNCTION(BlueprintCallable, Category = "Symptoms")
    static bool GetDeformationVisual(const TInstancedStruct<FVisualBase>& Visual, FVisualDeformation& OutDeformation);
};