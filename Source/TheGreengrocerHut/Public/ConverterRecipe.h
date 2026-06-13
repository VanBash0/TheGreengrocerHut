#pragma once

#include "CoreMinimal.h"
#include "IngredientStructures.h"
#include "ConverterRecipe.generated.h"

USTRUCT(BlueprintType)
struct FConverterRecipe
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FIngredient From;

    UPROPERTY(BlueprintReadWrite)
    FIngredient To;
    UPROPERTY(BlueprintReadWrite)
    int UseAmount = 0;
};