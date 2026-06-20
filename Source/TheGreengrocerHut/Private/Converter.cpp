#include "Converter.h"

bool UConverter::ContainsRecipe(FName IngredientRowName) {
    for (const auto& recipe : RecipeArray) {
        if (recipe.From.RowName == IngredientRowName) {
            return true;
        }
    }
    return false;
}

FConverterRecipe UConverter::GetRecipe(FName IngredientRowName) {
    if (ContainsRecipe(IngredientRowName)) {
        for (const auto& recipe : RecipeArray) {
            if (recipe.From.RowName == IngredientRowName) {
                return recipe;
            }
        }
    }
    return { NAME_None, NAME_None, 0 };
}