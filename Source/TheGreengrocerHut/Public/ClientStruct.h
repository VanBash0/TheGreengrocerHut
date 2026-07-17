#pragma once
#include "CoreMinimal.h"
#include "ClientStruct.generated.h" 

USTRUCT(BlueprintType)
struct FClient
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FName> Symptoms;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool IsDemon = false;
};

UENUM(BlueprintType)
enum class EPotionResult : uint8
{
    Human_Healed,
    Human_NotHealed,
    Human_Poisoned,

    Demon_Poisoned,
    Demon_NotPoisoned,
    Demon_GivenGoodPotion
};

// „истый результат сравнени€ поданного зель€ с каноническим рецептом.
// Ќе знает ничего про демонов/людей/дельты заражени€ Ч только "что дали" и "насколько верно".
USTRUCT(BlueprintType)
struct FPotionCompareResult
{
    GENERATED_BODY()

    // –€д ингредиентов, которые фактически дал игрок (индекс 0 Ч база), в виде RowName
    UPROPERTY(BlueprintReadOnly, Category = "Potion Compare")
    TArray<FName> GivenIngredients;

    // ¬алидность каждого ингредиента из GivenIngredients (по индексу, дл€ подсветки в UI).
    // ¬алидность i>0 считаетс€ только если весь рецепт совпал целиком (см. PotionMatchScore == 1.0).
    UPROPERTY(BlueprintReadOnly, Category = "Potion Compare")
    TArray<bool> IngredientValidity;

    // Ќасколько поданные ингредиенты (включа€ базу первым слотом) совпадают с канонической
    // последовательностью: база + нужные по симптомам, сгруппированные по AddPriority.
    // ћежду группами пор€док строго важен (блок закрываетс€ прежде чем начнЄтс€ следующий),
    // ¬Ќ”“–» группы с одинаковым приоритетом пор€док не важен (сравнение как мультимножество).
    // 0.0 Ч ни одна позици€ не совпала, 1.0 Ч точное совпадение рецепта целиком.
    UPROPERTY(BlueprintReadOnly, Category = "Potion Compare")
    float PotionMatchScore = 0.f;

    // ƒол€ валидных ингредиентов (matchingIngredients / Ingredients.Num()) по итогам прохода
    // проверки пор€дка приоритетов. √ейтитс€ через PotionMatchScore == 1.0 дл€ i>0 Ч см. IngredientValidity.
    UPROPERTY(BlueprintReadOnly, Category = "Potion Compare")
    float ValidFraction = 0.f;

    // ѕервый ингредиент (база) Ч это отрава
    UPROPERTY(BlueprintReadOnly, Category = "Potion Compare")
    bool IsPoison = false;
};

USTRUCT(BlueprintType)
struct FClientResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Client Result")
    EPotionResult Result = EPotionResult::Human_NotHealed;

    UPROPERTY(BlueprintReadOnly, Category = "Client Result")
    float DeltaInfectionRate = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Client Result")
    FClient Client;

    UPROPERTY(BlueprintReadOnly, Category = "Client Result")
    TArray<FName> GivenIngredients;

    UPROPERTY(BlueprintReadOnly, Category = "Client Result")
    TArray<bool> IngredientValidity;

    UPROPERTY(BlueprintReadOnly, Category = "Client Result")
    float PotionMatchScore = 0.f;
};

USTRUCT(BlueprintType)
struct FTutorialDay : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FClient> Clients;
};