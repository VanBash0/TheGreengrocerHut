#include "SymptomFunctionLibary.h"

bool USymptomFunctionLibary::GetOverlayVisual(const TInstancedStruct<FVisualBase>& Visual, FVisualOverlay& OutOverlay)
{
    if (const FVisualOverlay* Overlay = Visual.GetPtr<FVisualOverlay>())
    {
        OutOverlay = *Overlay;
        return true;
    }
    return false;
}

bool USymptomFunctionLibary::GetDeformationVisual(const TInstancedStruct<FVisualBase>& Visual, FVisualDeformation& OutDeformation)
{
    if (const FVisualDeformation* Deform = Visual.GetPtr<FVisualDeformation>())
    {
        OutDeformation = *Deform;
        return true;
    }
    return false;
}