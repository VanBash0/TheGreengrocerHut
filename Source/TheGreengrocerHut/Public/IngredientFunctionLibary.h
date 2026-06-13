#pragma once

#include "CoreMinimal.h"
#include "IngredientStructures.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "IngredientFunctionLibary.generated.h"

UCLASS(BlueprintType)
class UIngredientFunctionLibary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Ingredient")
    static void GetTwoStrongestColors(const TArray<FIngredient>& Ingredients, FLinearColor& OutColor1, FLinearColor& OutColor2);
};
