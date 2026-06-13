#include "IngredientFunctionLibary.h"

void UIngredientFunctionLibary::GetTwoStrongestColors(const TArray<FIngredient>& Ingredients, FLinearColor& OutColor1, FLinearColor& OutColor2)
{
    OutColor1 = FLinearColor::Black;
    OutColor2 = FLinearColor::Black;
    float MaxAlpha1 = -1.0f;
    float MaxAlpha2 = -1.0f;

    for (const FIngredient& Ingredient : Ingredients)
    {
        float A = Ingredient.MainColor.A;

        if (A > MaxAlpha1)
        {
            OutColor2 = OutColor1;
            MaxAlpha2 = MaxAlpha1;

            OutColor1 = Ingredient.MainColor;
            MaxAlpha1 = A;
        }
        else if (A > MaxAlpha2)
        {
            OutColor2 = Ingredient.MainColor;
            MaxAlpha2 = A;
        }
    }

    OutColor1.A = 1.0f;
    OutColor2.A = 1.0f;
}