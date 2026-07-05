#pragma once

#include "CoreMinimal.h"

#include "Subsystems/GameInstanceSubsystem.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"
#include "AssetRegistry/ARFilter.h"

#include "IngredientStructures.h"
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

private:
    void PopulateConverterCache();
    UPROPERTY()
    TArray<TObjectPtr<UConverter>> _converterCache;
    FString _converterFolderPath;
    bool _converterCacheLoaded = false;

public:
    const TArray<TObjectPtr<UConverter>>& GetConverterCache();
    const UConverter* GetConverterByIndex(int32 Index);

private:
    void BuildIngredientHashCache();
    TMap<uint64, TArray<FName>> _ingredientHashToRowName;

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

public:
    const FName GetRowNameByIngredient(const FIngredient& Ingredient) const;
};

UCLASS(BlueprintType)
class UIngredientFunctionLibary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Ingredient")
    static void GetTwoStrongestColors(const TArray<FIngredient>& Ingredients, FLinearColor& OutColor1, FLinearColor& OutColor2);

    UFUNCTION(BlueprintCallable, Category = "Ingredient")
    static void SelectUnlockedRecipe(const TArray<FName>& IngredientNames, const TArray<FConverterRecipe>& AllRecieps, TArray<FConverterRecipe>& OutUnclockedRecieps);

    UFUNCTION(BlueprintCallable, Category = "Potion", meta = (WorldContext = "WorldContextObject"))
    static void GetBasePotions(const UObject* WorldContextObject, const int& MaxClientSymptomCount, const bool& HasDemonAppeared, TArray<FName>& BasePotions);

    UFUNCTION(BlueprintCallable, Category = "Ingredient", meta = (WorldContext = "WorldContextObject"))
    static void GetDefaultIngredients(const UObject* WorldContextObject, const TArray<FName>& Ingredients, TArray<FName>& DefaultIngredients);

    UFUNCTION(BlueprintCallable, Category = "Potion", meta = (WorldContext = "WorldContextObject"))
    static void CalculatePotionQuality(const UObject* WorldContextObject, const TArray<FIngredient>& Ingredients,
                                       const UGameLoop* GameLoop, bool& IsGood, float& NewInfectionRate);
public:
    UFUNCTION(BlueprintCallable, Category = "Cache|Ingredients", meta = (WorldContext = "WorldContextObject"))
    static TArray<FIngredient> GetAllIngredients(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "Cache|Ingredients", meta = (WorldContext = "WorldContextObject"))
    static const FIngredient& GetIngredientByRowName(const UObject* WorldContextObject, FName RowName, bool& bFound);

    UFUNCTION(BlueprintCallable, Category = "Cache|Ingredients", meta = (WorldContext = "WorldContextObject"))
    static const FIngredient& GetIngredientByIndex(const UObject* WorldContextObject, int32 Index, bool& bFound);

    UFUNCTION(BlueprintCallable, Category = "Cache|Ingredients", meta = (WorldContext = "WorldContextObject"))
    static const FName GetRowNameByIngredient(const UObject* WorldContextObject, const FIngredient& Ingredient, bool& bFound);

    UFUNCTION(BlueprintCallable, Category = "Cache|Symptoms", meta = (WorldContext = "WorldContextObject"))
    static TArray<FSymptomRow> GetAllSymptoms(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "Cache|Symptoms", meta = (WorldContext = "WorldContextObject"))
    static const FSymptomRow& GetSymptomByRowName(const UObject* WorldContextObject, FName RowName, bool& bFound);

    UFUNCTION(BlueprintCallable, Category = "Cache|Symptoms", meta = (WorldContext = "WorldContextObject"))
    static const FSymptomRow& GetSymptomByIndex(const UObject* WorldContextObject, int32 Index, bool& bFound);

    UFUNCTION(BlueprintCallable, Category = "Cache|Converters", meta = (WorldContext = "WorldContextObject"))
    static TArray<UConverter*> GetAllConverters(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "Cache|Converters", meta = (WorldContext = "WorldContextObject"))
    static const UConverter* GetConverterByIndex(const UObject* WorldContextObject, int32 Index, bool& bFound);

public:
    UFUNCTION(BlueprintCallable, Category = "Cache|Ingredients", meta = (WorldContext = "WorldContextObject"))
    static void GetIngredientsBySymptoms(const UObject* WorldContextObject, const TArray<FName>& SymptomRowNames, TArray<FName>& OutIngredientRowNames);

public:
    UFUNCTION(BlueprintCallable, Category = "Tutorial", meta = (WorldContext = "WorldContextObject"))
    static int32 GetTutorialDaysNum(const UObject* WorldContextObject);

public:
    UFUNCTION(BlueprintCallable, Category = "Newspaper", meta = (WorldContext = "WorldContextObject"))
    static const FNewspaper BuildNewspaperFromSnapshot(const UObject* WorldContextObject, const FDaySnapshot& Snapshot, int32 DayNum);

    UFUNCTION(BlueprintCallable, Category = "Newspaper", meta = (WorldContext = "WorldContextObject"))
    static void GetAllNewspapers(const UObject* WorldContextObject, const TArray<FDaySnapshot>& PreviousDaysSnapshots, const FDaySnapshot& CurrentDaySnapshot, const bool CurrentDayIncluded, TArray<FNewspaper>& OutNewspapers);

private:
    static UCacheSubsystem* GetCacheSystem(const UObject* WorldContextObject);
};
