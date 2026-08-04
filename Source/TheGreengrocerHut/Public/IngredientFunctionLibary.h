#pragma once

#include "CoreMinimal.h"

#include "Subsystems/GameInstanceSubsystem.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"
#include "AssetRegistry/ARFilter.h"

#include "IngredientStructures.h"
#include "GardenStructures.h"
#include "SymptomStructures.h"
#include "Converter.h"
#include "ConverterRecipe.h"
#include "GameSettings.h"
#include "ClientStruct.h"
#include "GameLoop.h"
#include "NewsPaperStructures.h"

#include "IngredientFunctionLibary.generated.h"

UCLASS()
class UCacheSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // ============================================================
    // ИНГРЕДИЕНТЫ (кэш по FName, без хэша)
    // ============================================================
private:
    void PopulateIngredientCache();
    UPROPERTY()
    TObjectPtr<UDataTable> _ingredientTable;
    TMap<FName, FIngredient> _ingredientCache;
    bool _ingredientCacheLoaded = false;

public:
    const TMap<FName, FIngredient>& GetIngredientCache();
    const FIngredient* GetIngredientByRowName(FName rowName);
    const FIngredient* GetIngredientByIndex(int32 Index);

    // ============================================================
    // ИНГРЕДИЕНТЫ — ХЭШ-ИНДЕКС (обратный поиск: структура -> FName)
    // ============================================================
private:
    void BuildIngredientHashCache();
    TMap<uint64, TArray<FName>> _ingredientHashToRowName;

public:
    // Использует _ingredientHashToRowName (хэш по содержимому FIngredient)
    const FName GetRowNameByIngredient(const FIngredient& Ingredient) const;

    // ============================================================
    // СЕМЕНА ИНГРЕДИЕНТОВ (кэш по FName, без хэша)
    // ============================================================
private:
    void PopulateIngredientSeedCache();
    UPROPERTY()
    TObjectPtr<UDataTable> _ingredientSeedTable;
    TMap<FName, FIngredientSeed> _ingredientSeedCache;
    bool _ingredientSeedCacheLoaded = false;

public:
    const TMap<FName, FIngredientSeed>& GetIngredientSeedCache();
    const FIngredientSeed* GetIngredientSeedByRowName(FName rowName);

    // ============================================================
    // СИМПТОМЫ (кэш по FName, без хэша)
    // ============================================================
private:
    void PopulateSymptomCache();
    UPROPERTY()
    TObjectPtr<UDataTable> _symptomTable;
    TMap<FName, FSymptomRow> _symptomCache;
    bool _symptomCacheLoaded = false;

public:
    const TMap<FName, FSymptomRow>& GetSymptomCache();
    const FSymptomRow* GetSymptomByRowName(FName RowName);
    const FSymptomRow* GetSymptomByIndex(int32 Index);

    // ============================================================
    // КОНВЕРТЕРЫ / РЕЦЕПТЫ (кэш через AssetRegistry, без хэша)
    // ============================================================
private:
    void PopulateConverterCache();
    UPROPERTY()
    TArray<TObjectPtr<UConverter>> _converterCache;
    FString _converterFolderPath;
    bool _converterCacheLoaded = false;

public:
    const TArray<TObjectPtr<UConverter>>& GetConverterCache();
    const UConverter* GetConverterByIndex(int32 Index);

    // ============================================================
    // ГАЗЕТЫ / ТУТОРИАЛ (прямой доступ к DataTable, без хэша)
    // ============================================================
public:
    UDataTable* GetNewspaperTable() const { return _newspaperTable; }
    UDataTable* GetTutorialNewspaperTable() const { return _tutorialNewspaperTable; }
    UDataTable* GetTutorialDaysTable() const { return _tutorialDaysTable; }

private:
    UPROPERTY()
    TObjectPtr<UDataTable> _newspaperTable;
    UPROPERTY()
    TObjectPtr<UDataTable> _tutorialNewspaperTable;
    UPROPERTY()
    TObjectPtr<UDataTable> _tutorialDaysTable;
};

UCLASS(BlueprintType)
class UIngredientFunctionLibary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    // ============================================================
    // ИНГРЕДИЕНТЫ — без хэша (прямой доступ по FName / индексу)
    // ============================================================
public:
    UFUNCTION(BlueprintCallable, Category = "Ingredient")
    static void GetTwoStrongestColors(const TArray<FIngredient>& Ingredients, FLinearColor& OutColor1, FLinearColor& OutColor2);

    UFUNCTION(BlueprintCallable, Category = "Ingredient", meta = (WorldContext = "WorldContextObject", ToolTip = "По списку ингредиентов возвращает только сырые"))
    static void GetDefaultIngredients(const UObject* WorldContextObject, const TArray<FName>& Ingredients, TArray<FName>& DefaultIngredients);

    UFUNCTION(BlueprintCallable, Category = "Ingredient", meta = (WorldContext = "WorldContextObject", ToolTip = "Возвращает список ингредиентов, которые впервые появились сегодня"))
    static void GetNewIngredientsOfToday(const UObject* WorldContextObject, const UGameLoop* GameLoop, const TArray<FName>& TodayIngredients, TArray<FName>& NewIngredients);

    UFUNCTION(BlueprintCallable, Category = "Ingredient", meta = (WorldContext = "WorldContextObject", ToolTip = "По списку ингредиентов возвращает их сырые версии"))
    static void GetIngredientRawVariants(const UObject* WorldContextObject, const TArray<FName>& Ingredients, TArray<FName>& RawIngredients);

    UFUNCTION(BlueprintCallable, Category = "Ingredient", meta = (WorldContext = "WorldContextObject"))
    static void SortIngredientsByPriority(const UObject* WorldContextObject, UPARAM(ref) TArray<FName>& IngredientNames, bool bDescending = true);

    UFUNCTION(BlueprintCallable, Category = "Cache|Ingredients", meta = (WorldContext = "WorldContextObject"))
    static TArray<FIngredient> GetAllIngredients(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "Cache|Ingredients", meta = (WorldContext = "WorldContextObject"))
    static const FIngredient& GetIngredientByRowName(const UObject* WorldContextObject, FName RowName, bool& bFound);

    UFUNCTION(BlueprintCallable, Category = "Cache|Ingredients", meta = (WorldContext = "WorldContextObject"))
    static const FIngredient& GetIngredientByIndex(const UObject* WorldContextObject, int32 Index, bool& bFound);

    // ============================================================
    // ИНГРЕДИЕНТЫ — С ХЭШЕМ (ComputeIngredientHash / _ingredientHashToRowName)
    // ============================================================

    // Обратный поиск: структура FIngredient -> FName.
    // Строит хэш по содержимому (цвет, ассеты, приоритет) и ищет в UCacheSubsystem::_ingredientHashToRowName.
    UFUNCTION(BlueprintCallable, Category = "Cache|Ingredients", meta = (WorldContext = "WorldContextObject"))
    static const FName GetRowNameByIngredient(const UObject* WorldContextObject, const FIngredient& Ingredient, bool& bFound);

    // Косвенно использует хэш: внутри цикла вызывает GetRowNameByIngredient для каждого ингредиента игрока
    UFUNCTION(BlueprintCallable, Category = "Potion", meta = (WorldContext = "WorldContextObject"))
    static void ComparePotionToRecipe(const UObject* WorldContextObject, const TArray<FIngredient>& Ingredients, const UGameLoop* GameLoop, FPotionCompareResult& OutCompare);

    UFUNCTION(BlueprintCallable, Category = "Potion", meta = (WorldContext = "WorldContextObject"))
    static void CalculateClientResult(const UObject* WorldContextObject, const TArray<FIngredient>& Ingredients, const UGameLoop* GameLoop, FClientResult& OutResult);

    // ============================================================
    // СЕМЕНА ИНГРЕДИЕНТОВ — без хэша
    // ============================================================
public:
    UFUNCTION(BlueprintCallable, Category = "Cache|IngredientsSeed", meta = (WorldContext = "WorldContextObject"))
    static void GetIngredientSeedByIngredient(const UObject* WorldContextObject, const TArray<FName>& IngredientRowNames, TArray<FName>& OutSeedIngredientRowNames);

    // ============================================================
    // СИМПТОМЫ — без хэша
    // ============================================================
public:
    UFUNCTION(BlueprintCallable, Category = "Symptom", meta = (WorldContext = "WorldContextObject"))
    static TArray<FName> SelectNewSymptoms(const UObject* WorldContextObject, const TArray<FDaySnapshot>& PreviousDaysSnapshot, const FDaySnapshot& CurrentDaySnapshot);

    UFUNCTION(BlueprintCallable, Category = "Cache|Symptoms", meta = (WorldContext = "WorldContextObject"))
    static TArray<FSymptomRow> GetAllSymptoms(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "Cache|Symptoms", meta = (WorldContext = "WorldContextObject"))
    static const FSymptomRow& GetSymptomByRowName(const UObject* WorldContextObject, FName RowName, bool& bFound);

    UFUNCTION(BlueprintCallable, Category = "Cache|Symptoms", meta = (WorldContext = "WorldContextObject"))
    static const FSymptomRow& GetSymptomByIndex(const UObject* WorldContextObject, int32 Index, bool& bFound);

    UFUNCTION(BlueprintCallable, Category = "Cache|Ingredients", meta = (WorldContext = "WorldContextObject"))
    static void GetIngredientsBySymptoms(const UObject* WorldContextObject, const TArray<FName>& SymptomRowNames, TArray<FName>& OutIngredientRowNames);

    // ============================================================
    // ЗЕЛЬЯ / ПОТИОНЫ — без хэша
    // ============================================================
public:
    UFUNCTION(BlueprintCallable, Category = "Potion", meta = (WorldContext = "WorldContextObject"))
    static void GetBasePotions(const UObject* WorldContextObject, const int& MaxClientSymptomCount, const bool& HasDemonAppeared, TArray<FName>& BasePotions);

    UFUNCTION(BlueprintCallable, Category = "Ingredient", meta = (WorldContext = "WorldContextObject"))
    static void GetAllIngredientsForDay(const UObject* WorldContextObject, const UGameLoop* GameLoop, TArray<FName>& IngredientNames);

    UFUNCTION(BlueprintCallable, Category = "Ingredient", meta = (WorldContext = "WorldContextObject"))
    static void GetBasePotionBySumptomCount(const UObject* WorldContextObject, const UGameSettings* GameSettings, const int& ClientSymptomCount, FName& Potion);

    // ============================================================
    // КОНВЕРТЕРЫ / РЕЦЕПТЫ — без хэша
    // ============================================================
public:
    UFUNCTION(BlueprintCallable, Category = "Ingredient")
    static void SelectUnlockedRecipe(const TArray<FName>& IngredientNames, const TArray<FConverterRecipe>& AllRecieps, TArray<FConverterRecipe>& OutUnclockedRecieps);

    UFUNCTION(BlueprintCallable, Category = "Cache|Converters", meta = (WorldContext = "WorldContextObject"))
    static TArray<UConverter*> GetAllConverters(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "Cache|Converters", meta = (WorldContext = "WorldContextObject"))
    static const UConverter* GetConverterByIndex(const UObject* WorldContextObject, int32 Index, bool& bFound);

    // ============================================================
    // ГАЗЕТЫ — без хэша
    // ============================================================
public:
    UFUNCTION(BlueprintCallable, Category = "Newspaper", meta = (WorldContext = "WorldContextObject"))
    static const FNewspaper BuildNewspaperFromSnapshot(const UObject* WorldContextObject, const FDaySnapshot& Snapshot, int32 DayNum);

    UFUNCTION(BlueprintCallable, Category = "Newspaper", meta = (WorldContext = "WorldContextObject"))
    static void GetAllNewspapers(const UObject* WorldContextObject, const TArray<FDaySnapshot>& PreviousDaysSnapshots, const FDaySnapshot& CurrentDaySnapshot, const bool CurrentDayIncluded, TArray<FNewspaper>& OutNewspapers);

    // ============================================================
    // ТУТОРИАЛ — без хэша
    // ============================================================
public:
    UFUNCTION(BlueprintCallable, Category = "Tutorial", meta = (WorldContext = "WorldContextObject"))
    static int32 GetTutorialDaysNum(const UObject* WorldContextObject);

    // ============================================================
    // СЛУЖЕБНОЕ
    // ============================================================
private:
    static UCacheSubsystem* GetCacheSystem(const UObject* WorldContextObject);
};