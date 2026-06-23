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
        _converterFolderPath = ProjectSettings->ConverterFolderPath;
    }

    PopulateIngredientCache();
    PopulateSymptomCache();
    PopulateConverterCache();
}

void UCacheSubsystem::Deinitialize()
{
    _ingredientTable = nullptr;
    _ingredientCache.Empty();
    _ingredientCacheLoaded = false;

    _symptomTable = nullptr;
    _symptomCache.Empty();
    _symptomCacheLoaded = false;

    _converterCache.Empty();
    _converterCacheLoaded = false;

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

void UCacheSubsystem::PopulateConverterCache()
{
    if (_converterCacheLoaded) { return; }

    _converterCache.Empty();

    UAssetManager& AssetManager = UAssetManager::Get();
    TArray<FAssetData> AssetDataList;

    FARFilter Filter;
    Filter.PackagePaths.Add(FName(*_converterFolderPath));
    Filter.ClassPaths.Add(UConverter::StaticClass()->GetClassPathName());
    Filter.bRecursiveClasses = true;

    IAssetRegistry& AssetRegistry = AssetManager.GetAssetRegistry();
    AssetRegistry.GetAssets(Filter, AssetDataList);

    for (const FAssetData& AssetData : AssetDataList)
    {
        UConverter* Converter = Cast<UConverter>(AssetData.GetAsset());
        if (Converter)
        {
            _converterCache.Add(Converter);
        }
    }

    _converterCacheLoaded = true;
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

const FIngredient* UCacheSubsystem::GetIngredientByIndex(int32 Index)
{
    if (!_ingredientCacheLoaded) { PopulateIngredientCache(); }
    if (Index < 0 || Index >= _ingredientCache.Num()) { return nullptr; }

    auto It = _ingredientCache.CreateConstIterator();
    for (int32 i = 0; i < Index; ++i) { ++It; }

    return &It->Value;
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

const FSymptomRow* UCacheSubsystem::GetSymptomByIndex(int32 Index)
{
    if (!_symptomCacheLoaded) { PopulateSymptomCache(); }
    if (Index < 0 || Index >= _symptomCache.Num()) { return nullptr; }

    auto It = _symptomCache.CreateConstIterator();
    for (int32 i = 0; i < Index; ++i) { ++It; }

    return &It->Value;
}

//GETTERS_CONVERTER
const TArray<TObjectPtr<UConverter>>& UCacheSubsystem::GetConverterCache()
{
    if (!_converterCacheLoaded)
    {
        PopulateConverterCache();
    }

    return _converterCache;
}

const UConverter* UCacheSubsystem::GetConverterByIndex(int32 Index)
{
    if (!_converterCacheLoaded) { PopulateConverterCache(); }

    if (!_converterCache.IsValidIndex(Index)) { return nullptr; }

    return _converterCache[Index].Get();
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

void UIngredientFunctionLibary::SelectUnlockedRecipe(const TArray<FName>& IngredientNames, const TArray<FConverterRecipe>& AllRecieps, TArray<FConverterRecipe>& OutUnclockedRecieps)
{
    OutUnclockedRecieps.Empty();

    for (const FConverterRecipe& Recipe : AllRecieps)
    {
        if (IngredientNames.Contains(Recipe.To.RowName))
        {
            OutUnclockedRecieps.Add(Recipe);
        }
    }
}

void UIngredientFunctionLibary::GetIngredientsBySymptoms(const UObject* WorldContextObject, const TArray<FName>& SymptomRowNames, TArray<FName>& OutIngredientRowNames)
{
    OutIngredientRowNames.Empty();

    UCacheSubsystem* Cache = GetCacheSystem(WorldContextObject);
    if (!Cache) return;

    const TMap<FName, FSymptomRow>& Symptoms = Cache->GetSymptomCache();

    TSet<FName> ResultSet;

    for (const FName& SymptomName : SymptomRowNames)
    {
        const FSymptomRow* Symptom = Symptoms.Find(SymptomName);
        if (!Symptom) continue;

        const FName& IngredientRowName = Symptom->IngredientRow.RowName;
        if (!IngredientRowName.IsNone())
        {
            ResultSet.Add(IngredientRowName);
        }
    }

    OutIngredientRowNames = ResultSet.Array();
}

TArray<FIngredient> UIngredientFunctionLibary::GetAllIngredients(const UObject* WorldContextObject)
{
    UCacheSubsystem* Cache = GetCacheSystem(WorldContextObject);
    if (!Cache) return {};

    TArray<FIngredient> Result;
    Cache->GetIngredientCache().GenerateValueArray(Result);
    return Result;
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

const FIngredient& UIngredientFunctionLibary::GetIngredientByIndex(const UObject* WorldContextObject, int32 Index, bool& bFound)
{
    static FIngredient Empty;
    UCacheSubsystem* Cache = GetCacheSystem(WorldContextObject);
    if (!Cache) { bFound = false; return Empty; }

    const FIngredient* Result = Cache->GetIngredientByIndex(Index);
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

const FSymptomRow& UIngredientFunctionLibary::GetSymptomByRowName(const UObject* WorldContextObject, FName RowName, bool& bFound)
{
    static FSymptomRow Empty;
    UCacheSubsystem* Cache = GetCacheSystem(WorldContextObject);
    if (!Cache) { bFound = false; return Empty; }

    const FSymptomRow* Result = Cache->GetSymptomByRowName(RowName);
    bFound = Result != nullptr;
    return Result ? *Result : Empty;
}

const FSymptomRow& UIngredientFunctionLibary::GetSymptomByIndex(const UObject* WorldContextObject, int32 Index, bool& bFound)
{
    static FSymptomRow Empty;
    UCacheSubsystem* Cache = GetCacheSystem(WorldContextObject);
    if (!Cache) { bFound = false; return Empty; }

    const FSymptomRow* Result = Cache->GetSymptomByIndex(Index);
    bFound = Result != nullptr;

    return Result ? *Result : Empty;
}

TArray<UConverter*> UIngredientFunctionLibary::GetAllConverters(const UObject* WorldContextObject)
{
    UCacheSubsystem* Cache = GetCacheSystem(WorldContextObject);
    if (!Cache) return {};

    TArray<UConverter*> Result;
    for (const TObjectPtr<UConverter>& Converter : Cache->GetConverterCache())
    {
        if (Converter) { Result.Add(Converter.Get()); }
    }

    return Result;
}

const UConverter* UIngredientFunctionLibary::GetConverterByIndex(const UObject* WorldContextObject, int32 Index, bool& bFound)
{
    UCacheSubsystem* Cache = GetCacheSystem(WorldContextObject);
    if (!Cache) { bFound = false; return nullptr; }

    const UConverter* Result = Cache->GetConverterByIndex(Index);
    bFound = Result != nullptr;

    return Result;
}

void UIngredientFunctionLibary::GetBasePotions(const int& MaxClientSymptomCount,
                                               const bool& HasDemonAppeared,
                                               const TMap<int, FIngredientRowNameRef>& BasePotionsMap,
                                               const FIngredientRowNameRef& PoisonBase,
                                               TArray<FName>& BasePotions)
{
    for (const auto& base : BasePotionsMap) {
        if (MaxClientSymptomCount >= base.Key) {
            BasePotions.Add(base.Value.RowName);
        }
    }

    if (HasDemonAppeared) {
        BasePotions.Add(PoisonBase.RowName);
    }
}