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
    TMap<FName, FIngredient> _ingredientCache;
    TObjectPtr<UDataTable> _ingredientTable;
    bool _ingredientCacheLoaded = false;

public:
    const TMap<FName, FIngredient>& GetIngredientCache();
    const FIngredient* GetIngredientByRowName(FName rowName);
    const FIngredient* GetIngredientByIndex(int32 Index);

private:
    void PopulateSymptomCache();
    TMap<FName, FSymptomRow> _symptomCache;
    TObjectPtr<UDataTable> _symptomTable;
    bool _symptomCacheLoaded = false;

public:
    const TMap<FName, FSymptomRow>& GetSymptomCache();
    const FSymptomRow* GetSymptomByRowName(FName RowName);
    const FSymptomRow* GetSymptomByIndex(int32 Index);

private:
    void PopulateConverterCache();
    TArray<TObjectPtr<UConverter>> _converterCache;
    FString _converterFolderPath;
    bool _converterCacheLoaded = false;

public:
    const TArray<TObjectPtr<UConverter>>& GetConverterCache();
    const UConverter* GetConverterByIndex(int32 Index);
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

    UFUNCTION(BlueprintCallable, Category = "Ingredient")
    static void GetBasePotions(const int& MaxClientSymptomCount, const bool& HasDemonPrevious, const TMap<int, FIngredientRowNameRef>& BasePotionsMap, const FIngredientRowNameRef& PoisonBase, TArray<FName>& BasePotions);

    UFUNCTION(BlueprintCallable, Category = "Ingredient", meta = (WorldContext = "WorldContextObject"))
    static void GetDefaultIngredients(const UObject* WorldContextObject, const TArray<FName>& Ingredients, TArray<FName>& DefaultIngredients);
public:
    UFUNCTION(BlueprintCallable, Category = "Cache|Ingredients", meta = (WorldContext = "WorldContextObject"))
    static TArray<FIngredient> GetAllIngredients(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "Cache|Ingredients", meta = (WorldContext = "WorldContextObject"))
    static const FIngredient& GetIngredientByRowName(const UObject* WorldContextObject, FName RowName, bool& bFound);

    UFUNCTION(BlueprintCallable, Category = "Cache|Ingredients", meta = (WorldContext = "WorldContextObject"))
    static const FIngredient& GetIngredientByIndex(const UObject* WorldContextObject, int32 Index, bool& bFound);

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

private:
    static UCacheSubsystem* GetCacheSystem(const UObject* WorldContextObject);
};
