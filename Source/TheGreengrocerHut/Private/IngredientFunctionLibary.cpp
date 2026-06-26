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

        ProjectSettings->TutorialNewspaperDataTable.LoadSynchronous();
        ProjectSettings->NewspaperDataTable.LoadSynchronous();
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
    BuildIngredientHashCache();
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

void UIngredientFunctionLibary::GetBasePotions(const UObject* WorldContextObject,
                                               const int& MaxClientSymptomCount,
                                               const bool& HasDemonPrevious,
                                               TArray<FName>& BasePotions)
{
    BasePotions.Empty();
    const UGameProjectSettings* projectSettings = GetDefault<UGameProjectSettings>();
    const auto& BasePotionsMap = projectSettings->GameSettingsAsset->BasePotionsMap;
    const auto& PoisonBase = projectSettings->GameSettingsAsset->PoisonBase;

    for (const auto& base : BasePotionsMap) {
        if (MaxClientSymptomCount >= base.Key) {
            BasePotions.Add(base.Value.RowName);
        }
    }

    if (HasDemonPrevious) {
        BasePotions.Add(PoisonBase.RowName);
    }
}

void UIngredientFunctionLibary::GetDefaultIngredients(const UObject* WorldContextObject, const TArray<FName>& Ingredients, TArray<FName>& DefaultIngredients)
{
    const TArray<UConverter*> converters = GetAllConverters(WorldContextObject);

    TSet<FName> ToNames;

    for (const UConverter* converter : converters)
    {
        if (!converter) continue;
        for (const FConverterRecipe& recipe : converter->RecipeArray) {
            ToNames.Add(recipe.To.RowName);
        }
    }

    DefaultIngredients.Empty();

    for (const FName& ingredient : Ingredients) {
        if (!ToNames.Contains(ingredient)) {
            DefaultIngredients.Add(ingredient);
        }
    }
}

void UIngredientFunctionLibary::CalculatePotionQuality(const UObject* WorldContextObject,
                                                       const TArray<FIngredient>& Ingredients,
                                                       const UGameLoop* GameLoop,
                                                       bool& IsGood,
                                                       float& DeltaInfectionRate)
{
    TArray<FName> neededIngredients;
    FClient currentClient;
    GameLoop->GetCurrentClient(currentClient);
    GetIngredientsBySymptoms(WorldContextObject, currentClient.Symptoms, neededIngredients);
    auto gameSettings = GameLoop->GameSettings;
    
    int symptomsNum = currentClient.Symptoms.Num();
    int key = 0;
    for (const auto& base : gameSettings->BasePotionsMap) {
        if (symptomsNum >= base.Key) {
            key = FMath::Max(key, base.Key);
        }
    }
    neededIngredients.Add(gameSettings->BasePotionsMap[key].RowName);

    bool isPoison = false;
    int matchingIngredients = 0;
    int currentPriority = TNumericLimits<int32>::Min();
    for (const auto& ingredient : Ingredients) {
        bool bFound = false;
        FName ingredientName = GetRowNameByIngredient(WorldContextObject, ingredient, bFound);
        if (bFound && neededIngredients.Contains(ingredientName) && ingredient.TermsOfUse.AddPriority >= currentPriority) {
            matchingIngredients++;
            currentPriority = FMath::Max(currentPriority, ingredient.TermsOfUse.AddPriority);
            if (ingredientName == gameSettings->PoisonBase.RowName) {
                isPoison = true;
            }
        }
    }

    float fraction = matchingIngredients / Ingredients.Num();
    IsGood = (currentClient.IsDemon) ? isPoison : (fraction >= 0.5f && !isPoison);

    FGameMetrics gameMetrics;
    GameLoop->GetGameMetrics(gameMetrics);
    int dayMultiplier = 1 + 0.02f * (gameMetrics.DayNumber - 1);
    if (currentClient.IsDemon) {
        if (isPoison) {
            DeltaInfectionRate = -1 * gameSettings->BasicDeltaPoisonDemon * dayMultiplier * gameMetrics.HealingFactor;
        }
        else {
            DeltaInfectionRate = gameSettings->BasicDeltaNotPoisonDemon * dayMultiplier * gameMetrics.KillingFactor;
        }
    }
    else {
        if (isPoison) {
            DeltaInfectionRate = -1 * gameSettings->BasicDeltaPoisonClient * dayMultiplier * gameMetrics.HealingFactor;
        }
        else if (fraction >= 0.5f) {
            DeltaInfectionRate = gameSettings->BasicDeltaHeal * dayMultiplier * gameMetrics.HealingFactor * fraction;
        }
        else {
            DeltaInfectionRate = -1 * gameSettings->BasicDeltaNotHeal * dayMultiplier * (1 - fraction);
        }
    }
}

namespace {
    static uint64 ComputeIngredientHash(const FIngredient& Ingredient)
    {
        uint64 Hash = 0;

        // Name и DisplayName исключены из хэша, т.к. будут проблемы при локализации

        Hash = HashCombine(Hash, GetTypeHash(Ingredient.MainColor));

        auto GetAssetPath = [](const TObjectPtr<UObject>& Asset) -> FString
            {
                return Asset ? Asset->GetPathName() : FString();
            };

        Hash = HashCombine(Hash, GetTypeHash(GetAssetPath(Ingredient.Icon)));
        Hash = HashCombine(Hash, GetTypeHash(GetAssetPath(Ingredient.Mesh)));
        Hash = HashCombine(Hash, GetTypeHash(GetAssetPath(Ingredient.SFX)));
        Hash = HashCombine(Hash, GetTypeHash(GetAssetPath(Ingredient.Container)));
        Hash = HashCombine(Hash, GetTypeHash(Ingredient.TermsOfUse.AddPriority));
        Hash = HashCombine(Hash, GetTypeHash(Ingredient.TermsOfUse.Mixing));

        return Hash;
    }
}

void UCacheSubsystem::BuildIngredientHashCache()
{
    _ingredientHashToRowName.Empty();
    for (const auto& Pair : _ingredientCache) {
        const FName& RowName = Pair.Key;
        const FIngredient& Ingredient = Pair.Value;
        uint64 Hash = ComputeIngredientHash(Ingredient);
        _ingredientHashToRowName.FindOrAdd(Hash).Add(RowName);
    }
}

const FName UCacheSubsystem::GetRowNameByIngredient(const FIngredient& Ingredient) const
{
    if (!_ingredientCacheLoaded) {
        return NAME_None;
    }

    uint64 Hash = ComputeIngredientHash(Ingredient);
    const TArray<FName>* FoundNames = _ingredientHashToRowName.Find(Hash);
    if (!FoundNames) {
        return NAME_None;
    }

    if (FoundNames->Num() == 1) {
        return (*FoundNames)[0];
    }

    for (const FName& CandidateName : *FoundNames) {
        const FIngredient* Candidate = _ingredientCache.Find(CandidateName);
        if (Candidate) {
            if (FIngredient::StaticStruct()->CompareScriptStruct(&Ingredient, Candidate, 0) == 0) {
                return CandidateName;
            }
        }
    }

    return NAME_None;
}

const FName UIngredientFunctionLibary::GetRowNameByIngredient(const UObject* WorldContextObject, const FIngredient& Ingredient, bool& bFound)
{
    UCacheSubsystem* Cache = GetCacheSystem(WorldContextObject);
    if (!Cache) {
        bFound = false;
        return NAME_None;
    }

    FName Result = Cache->GetRowNameByIngredient(Ingredient);
    bFound = !Result.IsNone();
    return Result;
}

int32 UIngredientFunctionLibary::GetTutorialDaysNum(const UObject* WorldContextObject) {
    const UGameProjectSettings* projectSettings = GetDefault<UGameProjectSettings>();
    return projectSettings->TutorialDaysTable->GetRowMap().Num();
}

const FNewspaper UIngredientFunctionLibary::BuildNewspaperFromSnapshot(const UObject* WorldContextObject,
    const FDaySnapshot& Snapshot, int32 DayNum)
{
    FNewspaper outNewspaper;
    outNewspaper.DemonSymptoms = Snapshot.DemonSymptoms;

    const UGameProjectSettings* Settings = GetDefault<UGameProjectSettings>();
    if (!Settings) return outNewspaper;

    float rate = Snapshot.VillageInfectionRate;
    float rounded = FMath::Floor(FMath::Clamp(rate - FMath::Modulo(rate, 10.0f), -100.0f, 100.0f));
    FName rateRowName = FName(*FString::FromInt((int32)rounded));

    if (FNewspaperData* Data = Settings->NewspaperDataTable->FindRow<FNewspaperData>(rateRowName, TEXT(""))) {
        outNewspaper.VillageImage = Data->VillageImage;
        outNewspaper.Description = Data->Description;
    }
    else {
        outNewspaper.VillageImage = nullptr;
        outNewspaper.Description = FText::GetEmpty();
    }

    if (DayNum <= UIngredientFunctionLibary::GetTutorialDaysNum(WorldContextObject))
    {
        FName TutorRowName = FName(*FString::FromInt(DayNum));
        if (FTutorialNewspaperData* TutorData = Settings->TutorialNewspaperDataTable->FindRow<FTutorialNewspaperData>(TutorRowName, TEXT(""))) {
            outNewspaper.Description = TutorData->Description;
        }
    }

    return outNewspaper;
}

void UIngredientFunctionLibary::GetAllNewspapers(const UObject* WorldContextObject,
    const TArray<FDaySnapshot>& PreviousDaysSnapshots,
    const FDaySnapshot& CurrentDaySnapshot,
    const bool CurrentDayIncluded,
    TArray<FNewspaper>& OutNewspapers)
{
    int CurrentDay = PreviousDaysSnapshots.Num() + 1;
    for (int i = 0; i < CurrentDay - 1; ++i) {
        OutNewspapers.Add(BuildNewspaperFromSnapshot(WorldContextObject, PreviousDaysSnapshots[i], i + 1));
    }

    if (CurrentDayIncluded) {
        OutNewspapers.Add(BuildNewspaperFromSnapshot(WorldContextObject, CurrentDaySnapshot, CurrentDay));
    }
}