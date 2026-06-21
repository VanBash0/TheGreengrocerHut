#pragma once

#include "CoreMinimal.h"

#include "Subsystems/GameInstanceSubsystem.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "IngredientStructures.h"
#include "SymptomStructures.h"
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

private:
    void PopulateSymptomCache();
    TMap<FName, FSymptomRow> _symptomCache;
    TObjectPtr<UDataTable> _symptomTable;
    bool _symptomCacheLoaded = false;

public:
    const TMap<FName, FSymptomRow>& GetSymptomCache();
    const FSymptomRow* GetSymptomByRowName(FName RowName);
};

UCLASS(BlueprintType)
class UIngredientFunctionLibary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Ingredient")
    static void GetTwoStrongestColors(const TArray<FIngredient>& Ingredients, FLinearColor& OutColor1, FLinearColor& OutColor2);

public:
    UFUNCTION(BlueprintCallable, Category = "Cache|Ingredients", meta = (WorldContext = "WorldContextObject"))
    static const FIngredient& GetIngredientByRowName(const UObject* WorldContextObject, FName RowName, bool& bFound);

    UFUNCTION(BlueprintCallable, Category = "Cache|Ingredients", meta = (WorldContext = "WorldContextObject"))
    static TArray<FIngredient> GetAllIngredients(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "Cache|Symptoms", meta = (WorldContext = "WorldContextObject"))
    static const FSymptomRow& GetSymptomByRowName(const UObject* WorldContextObject, FName RowName, bool& bFound);

    UFUNCTION(BlueprintCallable, Category = "Cache|Symptoms", meta = (WorldContext = "WorldContextObject"))
    static TArray<FSymptomRow> GetAllSymptoms(const UObject* WorldContextObject);

private:
    static UCacheSubsystem* GetCacheSystem(const UObject* WorldContextObject);
};
