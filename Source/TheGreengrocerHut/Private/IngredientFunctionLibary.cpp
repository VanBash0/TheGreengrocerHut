#include "IngredientFunctionLibary.h"
#include "Kismet/GameplayStatics.h"

//SYSTEM
void UCacheSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    const UGameProjectSettings* ProjectSettings = GetDefault<UGameProjectSettings>();
    if (ProjectSettings)
    {
        _ingredientTable = ProjectSettings->IngredientTable.LoadSynchronous();
        _symptomTable = ProjectSettings->SymptomTable.LoadSynchronous();
    }

    PopulateIngredientCache();
    PopulateSymptomCache();
}

void UCacheSubsystem::Deinitialize()
{
    _ingredientTable = nullptr;
    _ingredientCache.Empty();
    _ingredientCacheLoaded = false;

    _symptomTable = nullptr;
    _symptomCache.Empty();
    _symptomCacheLoaded = false;

    Super::Deinitialize();
}

bool UCacheSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

//LOADING
void UCacheSubsystem::PopulateIngredientCache()
{
    if (!_ingredientTable || _ingredientCacheLoaded) { return; }

    _ingredientCache.Empty();
    static const FString Context(TEXT("UCacheSubsystem::PopulateIngredientCache"));
    TArray<FName> RowNames = _ingredientTable->GetRowNames();

    for (const FName& RowName : RowNames)
    {
        FIngredient* Row = _ingredientTable->FindRow<FIngredient>(RowName, Context);
        if (Row)
        {
            _ingredientCache.Add(RowName, *Row);
        }
    }

    _ingredientCacheLoaded = true;
}

void UCacheSubsystem::PopulateSymptomCache()
{
    if (!_symptomTable || _symptomCacheLoaded) { return; }

    _symptomCache.Empty();
    static const FString Context(TEXT("UCacheSubsystem::PopulateSymptomCache"));
    TArray<FName> RowNames = _symptomTable->GetRowNames();

    for (const FName& RowName : RowNames)
    {
        FSymptomRow* Row = _symptomTable->FindRow<FSymptomRow>(RowName, Context);
        if (Row)
        {
            _symptomCache.Add(RowName, *Row);
        }
    }

    _symptomCacheLoaded = true;
}

//GETTERS_INGREDIENT
const TMap<FName, FIngredient>& UCacheSubsystem::GetIngredientCache()
{
    if (!_ingredientCacheLoaded)
    {
        PopulateIngredientCache();
    }

    return _ingredientCache;
}

const FIngredient* UCacheSubsystem::GetIngredientByRowName(FName RowName)
{
    if (!_ingredientCacheLoaded)
    {
        PopulateIngredientCache();
    }

    return _ingredientCache.Find(RowName);
}

//GETTERS_SYMPTOM
const TMap<FName, FSymptomRow>& UCacheSubsystem::GetSymptomCache()
{
    if (!_symptomCacheLoaded)
    {
        PopulateSymptomCache();
    }

    return _symptomCache;
}

const FSymptomRow* UCacheSubsystem::GetSymptomByRowName(FName RowName)
{
    if (!_symptomCacheLoaded)
    {
        PopulateSymptomCache();
    }

    return _symptomCache.Find(RowName);
}

//LIBARY_MAIN
UCacheSubsystem* UIngredientFunctionLibary::GetCacheSystem(const UObject* WorldContextObject)
{
    static TWeakObjectPtr<UCacheSubsystem> CachedSubsystem;

    if (CachedSubsystem.IsValid())
    {
        return CachedSubsystem.Get();
    }

    UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject);
    if (!GI) { return nullptr; }

    CachedSubsystem = GI->GetSubsystem<UCacheSubsystem>();

    return CachedSubsystem.Get();
}

//LIBARY_FUNC
void UIngredientFunctionLibary::GetTwoStrongestColors(const TArray<FIngredient>& Ingredients, FLinearColor& OutColor1, FLinearColor& OutColor2)
{
    OutColor1 = FLinearColor::Black;
    OutColor2 = FLinearColor::Black;
    float MaxAlpha1 = -1.0f;
    float MaxAlpha2 = -1.0f;

    for (const FIngredient& Ingredient : Ingredients)
    {
        float A = Ingredient.MainColor.A;

        if (A > MaxAlpha1)
        {
            OutColor2 = OutColor1;
            MaxAlpha2 = MaxAlpha1;

            OutColor1 = Ingredient.MainColor;
            MaxAlpha1 = A;
        }
        else if (A > MaxAlpha2)
        {
            OutColor2 = Ingredient.MainColor;
            MaxAlpha2 = A;
        }
    }

    OutColor1.A = 1.0f;
    OutColor2.A = 1.0f;
}

const FIngredient& UIngredientFunctionLibary::GetIngredientByRowName(const UObject* WorldContextObject, FName RowName, bool& bFound)
{
    static FIngredient Empty;
    UCacheSubsystem* Cache = GetCacheSystem(WorldContextObject);
    if (!Cache) { bFound = false; return Empty; }

    const FIngredient* Result = Cache->GetIngredientByRowName(RowName);
    bFound = Result != nullptr;
    return Result ? *Result : Empty;
}

TArray<FIngredient> UIngredientFunctionLibary::GetAllIngredients(const UObject* WorldContextObject)
{
    UCacheSubsystem* Cache = GetCacheSystem(WorldContextObject);
    if (!Cache) return {};

    TArray<FIngredient> Result;
    Cache->GetIngredientCache().GenerateValueArray(Result);
    return Result;
}

const FSymptomRow& UIngredientFunctionLibary::GetSymptomByRowName(const UObject* WorldContextObject, FName RowName, bool& bFound)
{
    static FSymptomRow Empty;
    UCacheSubsystem* Cache = GetCacheSystem(WorldContextObject);
    if (!Cache) { bFound = false; return Empty; }

    const FSymptomRow* Result = Cache->GetSymptomByRowName(RowName);
    bFound = Result != nullptr;
    return Result ? *Result : Empty;
}

TArray<FSymptomRow> UIngredientFunctionLibary::GetAllSymptoms(const UObject* WorldContextObject)
{
    UCacheSubsystem* Cache = GetCacheSystem(WorldContextObject);
    if (!Cache) return {};

    TArray<FSymptomRow> Result;
    Cache->GetSymptomCache().GenerateValueArray(Result);
    return Result;
}
