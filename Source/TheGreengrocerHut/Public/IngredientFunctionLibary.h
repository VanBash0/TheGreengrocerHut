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
    UFUNCTION()
    static TArray<FName> GetIngredientRowNames()
    {
        TArray<FName> Names;
        UDataTable* Table = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_Ingredients.DT_Ingredients"));

        if (Table)
        {
            Names = Table->GetRowNames();
        }

        return Names;
    }

public:
    UFUNCTION(BlueprintCallable, Category = "Ingredient")
    static void GetTwoStrongestColors(const TArray<FIngredient>& Ingredients, FLinearColor& OutColor1, FLinearColor& OutColor2);
};
