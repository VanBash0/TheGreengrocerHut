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

// Чистый результат сравнения поданного зелья с каноническим рецептом.
// Не знает ничего про демонов/людей/дельты заражения — только "что дали" и "насколько верно".
USTRUCT(BlueprintType)
struct FPotionCompareResult
{
    GENERATED_BODY()

    // Ряд ингредиентов, которые фактически дал игрок (индекс 0 — база), в виде RowName
    UPROPERTY(BlueprintReadOnly, Category = "Potion Compare")
    TArray<FName> GivenIngredients;

    // Валидность каждого ингредиента из GivenIngredients (по индексу, для подсветки в UI).
    // Валидность i>0 считается только если весь рецепт совпал целиком (см. PotionMatchScore == 1.0).
    UPROPERTY(BlueprintReadOnly, Category = "Potion Compare")
    TArray<bool> IngredientValidity;

    // Насколько поданные ингредиенты (включая базу) совпадают с каноническим рецептом,
    // сгруппированным по AddPriority. Между уровнями приоритета порядок важен,
    // ВНУТРИ уровня — не важен (сравнение как мультимножество). 0.0..1.0
    UPROPERTY(BlueprintReadOnly, Category = "Potion Compare")
    float PotionMatchScore = 0.f;

    // Доля валидных ингредиентов по итогам проверки неубывающего порядка приоритетов.
    UPROPERTY(BlueprintReadOnly, Category = "Potion Compare")
    float ValidFraction = 0.f;

    // Первый ингредиент (база) — это отрава
    UPROPERTY(BlueprintReadOnly, Category = "Potion Compare")
    bool IsPoison = false;

    // Совпадение по каждому уровню приоритета отдельно — для UI-фидбека игроку.
    // Ключ = Priority (-1 База, 0 Основной, 1 Дополнительный, 2 Связывающий и т.д., см. GameSettings->IngredientPriorityData).
    // Значение = все ингредиенты игрока на этом уровне совпали (как мультимножество) с нужными.
    UPROPERTY(BlueprintReadOnly, Category = "Potion Compare")
    TMap<int32, bool> TierMatchResults;
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

    // Совпадение по каждому уровню приоритета — прокинуто из FPotionCompareResult,
    // чтобы в BP не пришлось отдельно дёргать ComparePotionToRecipe для UI-фидбека.
    // Ключ = Priority, значение = совпал ли этот уровень целиком.
    UPROPERTY(BlueprintReadOnly, Category = "Client Result")
    TMap<int32, bool> TierMatchResults;

    UPROPERTY(BlueprintReadOnly, Category = "Client Result")
    bool IsPotionGood = false;
};

USTRUCT(BlueprintType)
struct FTutorialDay : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FClient> Clients;
};