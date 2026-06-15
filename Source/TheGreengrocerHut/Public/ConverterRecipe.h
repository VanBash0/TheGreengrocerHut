#pragma once

#include "CoreMinimal.h"
#include "IngredientStructures.h"
#include "ConverterRecipe.generated.h"

USTRUCT(BlueprintType)
struct FConverterRecipe
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FIngredientRowNameRef From;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FIngredientRowNameRef To;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int UseAmount = 0;
};