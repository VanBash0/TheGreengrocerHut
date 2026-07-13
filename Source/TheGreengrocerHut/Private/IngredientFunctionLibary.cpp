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
        _ingredientSeedTable = ProjectSettings->IngredientSeedTable.LoadSynchronous();
        _symptomTable = ProjectSettings->SymptomTable.LoadSynchronous();
        _converterFolderPath = ProjectSettings->ConverterFolderPath;

        _newspaperTable = ProjectSettings->NewspaperDataTable.LoadSynchronous();
        _tutorialNewspaperTable = ProjectSettings->TutorialNewspaperDataTable.LoadSynchronous();
        _tutorialDaysTable = ProjectSettings->TutorialDaysTable.LoadSynchronous();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("doesn't load developer settings"));
        return;
    }

    PopulateIngredientCache();
    PopulateIngredientSeedCache();
    PopulateSymptomCache();
    PopulateConverterCache();
}

void UCacheSubsystem::Deinitialize()
{
    _ingredientTable = nullptr;
    _ingredientCache.Empty();
    _ingredientHashToRowName.Empty();
    _ingredientCacheLoaded = false;

    _ingredientSeedTable = nullptr;
    _ingredientSeedCache.Empty();
    _ingredientSeedCacheLoaded = false;

    _symptomTable = nullptr;
    _symptomCache.Empty();
    _symptomCacheLoaded = false;

    _converterCache.Empty();
    _converterCacheLoaded = false;

    _newspaperTable = nullptr;
    _tutorialNewspaperTable = nullptr;
    _tutorialDaysTable = nullptr;

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

void UCacheSubsystem::PopulateIngredientSeedCache()
{
    if (!_ingredientSeedTable || _ingredientSeedCacheLoaded) { return; }

    _ingredientSeedCache.Empty();
    static const FString Context(TEXT("UCacheSubsystem::PopulateIngredientSeedCache"));
    TArray<FName> RowNames = _ingredientSeedTable->GetRowNames();

    for (const FName& RowName : RowNames)
    {
        FIngredientSeed* Row = _ingredientSeedTable->FindRow<FIngredientSeed>(RowName, Context);
        if (Row)
        {
            _ingredientSeedCache.Add(RowName, *Row);
        }
    }

    _ingredientSeedCacheLoaded = true;
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

//GETTERS_INGREDIENT_SEED
const TMap<FName, FIngredientSeed>& UCacheSubsystem::GetIngredientSeedCache()
{
    if (!_ingredientSeedCacheLoaded)
    {
        PopulateIngredientSeedCache();
    }

    return _ingredientSeedCache;
}

const FIngredientSeed* UCacheSubsystem::GetIngredientSeedByRowName(FName RowName)
{
    if (!_ingredientSeedCacheLoaded)
    {
        PopulateIngredientSeedCache();
    }

    return _ingredientSeedCache.Find(RowName);
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

    TSet<FName> SeenSet;

    for (const FName& SymptomName : SymptomRowNames)
    {
        const FSymptomRow* Symptom = Symptoms.Find(SymptomName);
        if (!Symptom) continue;

        const FName& IngredientRowName = Symptom->IngredientRow.RowName;
        if (!IngredientRowName.IsNone() && !SeenSet.Contains(IngredientRowName))
        {
            SeenSet.Add(IngredientRowName);
            OutIngredientRowNames.Add(IngredientRowName);
        }
    }
}

void UIngredientFunctionLibary::GetIngredientSeedByIngredient(const UObject* WorldContextObject, const TArray<FName>& IngredientRowNames, TArray<FName>& OutSeedIngredientRowNames)
{
    OutSeedIngredientRowNames.Empty();

    UCacheSubsystem* Cache = GetCacheSystem(WorldContextObject);
    if (!Cache) return;

    const TMap<FName, FIngredientSeed>& Seeds = Cache->GetIngredientSeedCache();

    const TSet<FName> IngredientSet(IngredientRowNames);

    OutSeedIngredientRowNames.Reserve(IngredientSet.Num());

    for (const auto& Pair : Seeds)
    {
        const FName& SeedRowName = Pair.Key;
        const FIngredientSeed& Seed = Pair.Value;

        if (IngredientSet.Contains(Seed.GrowedIngredientRowName.RowName))
        {
            OutSeedIngredientRowNames.Add(SeedRowName);
        }
    }
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

void UIngredientFunctionLibary::GetBasePotionBySumptomCount(const UObject* WorldContextObject,
    const UGameSettings* GameSettings,
    const int& ClientSymptomCount,
    FName& Potion)
{
    int key = 0;
    for (const auto& base : GameSettings->BasePotionsMap)
    {
        if (ClientSymptomCount >= base.Key)
        {
            key = FMath::Max(key, base.Key);
        }
    }

    Potion = GameSettings->BasePotionsMap[key].RowName;
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

TArray<FName> UIngredientFunctionLibary::SelectNewSymptoms(const UObject* WorldContextObject, const TArray<FDaySnapshot>& PreviousDaysSnapshot, const FDaySnapshot& CurrentDaySnapshot)
{
    TArray<FName> NewSymptoms;
    NewSymptoms.Reserve(CurrentDaySnapshot.DaySymptoms.Num());

    if (PreviousDaysSnapshot.Num() == 0)
    {
        NewSymptoms = CurrentDaySnapshot.DaySymptoms;
        return NewSymptoms;
    }

    const FDaySnapshot& LastPreviousSnapshot = PreviousDaysSnapshot.Last();
    const TSet<FName> PreviousSymptomsSet(LastPreviousSnapshot.DaySymptoms);

    for (const FName& Symp : CurrentDaySnapshot.DaySymptoms)
    {
        if (!PreviousSymptomsSet.Contains(Symp))
        {
            NewSymptoms.Add(Symp);
        }
    }

    return NewSymptoms;
}

void UIngredientFunctionLibary::CalculatePotionQuality(const UObject* WorldContextObject,
    const TArray<FIngredient>& Ingredients,
    const UGameLoop* GameLoop,
    bool& IsGood,
    float& DeltaInfectionRate,
    TArray<FName>& OutIngredientNames,
    TArray<bool>& OutIngredientValidity)
{
    OutIngredientNames.Empty();
    OutIngredientValidity.Empty();
    OutIngredientNames.Reserve(Ingredients.Num());
    OutIngredientValidity.Reserve(Ingredients.Num());

    FClient currentClient;
    GameLoop->GetCurrentClient(currentClient);

    // ингредиенты по симптомам (без базы) — "правильный" набор для лечения
    TArray<FName> neededIngredients;
    GetIngredientsBySymptoms(WorldContextObject, currentClient.Symptoms, neededIngredients);

    const TObjectPtr<UGameSettings>& gameSettings = GameLoop->GameSettings;

    // требуемая база: демону — строго яд, иначе — по кол-ву симптомов
    FName requiredBase;
    if (currentClient.IsDemon)
    {
        requiredBase = gameSettings->PoisonBase.RowName;
    }
    else
    {
        GetBasePotionBySumptomCount(WorldContextObject, gameSettings, currentClient.Symptoms.Num(), requiredBase);
    }

    // резолвим имена игрока
    TArray<FName> playerIngredientNames;
    playerIngredientNames.Reserve(Ingredients.Num());
    for (const auto& ingredient : Ingredients)
    {
        bool bFound = false;
        FName ingredientName = GetRowNameByIngredient(WorldContextObject, ingredient, bFound);
        playerIngredientNames.Add(bFound ? ingredientName : NAME_None);
    }

    // 1. база первая и совпадает с требуемой
    const bool bBaseOk = !requiredBase.IsNone() && (playerIngredientNames[0] == requiredBase);

    // 2. остальные ингредиенты как мультимножество сравниваются с "правильным" набором по симптомам
    bool bRestMatchesSymptoms = false;
    if (bBaseOk)
    {
        TArray<FName> playerRest(playerIngredientNames);
        playerRest.RemoveAt(0);

        if (playerRest.Num() == neededIngredients.Num())
        {
            TArray<FName> playerRestSorted = playerRest;
            TArray<FName> neededSorted = neededIngredients;
            playerRestSorted.Sort([](const FName& A, const FName& B) { return A.LexicalLess(B); });
            neededSorted.Sort([](const FName& A, const FName& B) { return A.LexicalLess(B); });

            bRestMatchesSymptoms = (playerRestSorted == neededSorted);
        }
    }

    // демону "правильно" = база-яд + НЕправильный набор ингредиентов
    // обычному клиенту "правильно" = правильная база + правильный набор ингредиентов
    const bool bExactMatch = bBaseOk && (currentClient.IsDemon ? !bRestMatchesSymptoms : bRestMatchesSymptoms);

    // 3. проверка порядка по приоритету (после базы — неубывающий) + подсчёт валидных
    bool isPoison = false;
    int32 matchingIngredients = 0;
    int32 currentPriority = TNumericLimits<int32>::Min();

    for (int32 i = 0; i < Ingredients.Num(); ++i)
    {
        const FIngredient& ingredient = Ingredients[i];
        const FName ingredientName = playerIngredientNames[i];
        bool bIsValid = false;

        if (i == 0)
        {
            bIsValid = bBaseOk;
            if (bIsValid)
            {
                matchingIngredients++;
                currentPriority = TNumericLimits<int32>::Min();
                if (ingredientName == gameSettings->PoisonBase.RowName)
                {
                    isPoison = true;
                }
            }
        }
        else
        {
            bIsValid = bExactMatch
                && !ingredientName.IsNone()
                && ingredient.TermsOfUse.AddPriority >= currentPriority;

            if (bIsValid)
            {
                matchingIngredients++;
                currentPriority = FMath::Max(currentPriority, ingredient.TermsOfUse.AddPriority);
                if (ingredientName == gameSettings->PoisonBase.RowName)
                {
                    isPoison = true;
                }
            }
        }

        OutIngredientNames.Add(ingredientName);
        OutIngredientValidity.Add(bIsValid);
    }

    const float fraction = static_cast<float>(matchingIngredients) / static_cast<float>(Ingredients.Num());

    IsGood = (currentClient.IsDemon) ? isPoison : (fraction >= 0.5f && !isPoison);

    FGameMetrics gameMetrics;
    GameLoop->GetGameMetrics(gameMetrics);
    const float dayMultiplier = 1.f + 0.02f * (gameMetrics.DayNumber - 1);

    if (currentClient.IsDemon)
    {
        if (isPoison)
        {
            DeltaInfectionRate = -1.f * gameSettings->BasicDeltaPoisonDemon * dayMultiplier * gameMetrics.HealingFactor;
        }
        else
        {
            DeltaInfectionRate = gameSettings->BasicDeltaNotPoisonDemon * dayMultiplier * gameMetrics.KillingFactor;
        }
    }
    else
    {
        if (isPoison)
        {
            DeltaInfectionRate = -1.f * gameSettings->BasicDeltaPoisonClient * dayMultiplier * gameMetrics.HealingFactor;
        }
        else if (fraction >= 0.5f)
        {
            DeltaInfectionRate = gameSettings->BasicDeltaHeal * dayMultiplier * gameMetrics.HealingFactor * fraction;
        }
        else
        {
            DeltaInfectionRate = -1.f * gameSettings->BasicDeltaNotHeal * dayMultiplier * (1.f - fraction);
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

int32 UIngredientFunctionLibary::GetTutorialDaysNum(const UObject* WorldContextObject)
{
    UCacheSubsystem* Cache = GetCacheSystem(WorldContextObject);
    if (!Cache) return 0;

    UDataTable* DaysTable = Cache->GetTutorialDaysTable();
    if (!DaysTable) return 0;

    return DaysTable->GetRowNames().Num();
}

const FNewspaper UIngredientFunctionLibary::BuildNewspaperFromSnapshot(const UObject* WorldContextObject,
    const FDaySnapshot& Snapshot, int32 DayNum)
{
    FNewspaper outNewspaper;
    outNewspaper.DemonSymptoms = Snapshot.DemonSymptoms;

    UCacheSubsystem* Cache = GetCacheSystem(WorldContextObject);
    if (!Cache) return outNewspaper;

    UDataTable* NewspaperTable = Cache->GetNewspaperTable();
    if (!NewspaperTable) return outNewspaper;

    float rate = Snapshot.VillageInfectionRate;
    float rounded = FMath::Floor(FMath::Clamp(rate - FMath::Modulo(rate, 10.0f), -100.0f, 100.0f));
    FName rateRowName = FName(*FString::FromInt((int32)rounded));

    if (FNewspaperData* Data = NewspaperTable->FindRow<FNewspaperData>(rateRowName, TEXT("")))
    {
        outNewspaper.VillageImage = Data->VillageImage;
        outNewspaper.Description = Data->Description;
    }

    UDataTable* TutorialNewspaperTable = Cache->GetTutorialNewspaperTable();
    if (!TutorialNewspaperTable) return outNewspaper;

    if (DayNum <= UIngredientFunctionLibary::GetTutorialDaysNum(WorldContextObject))
    {
        FName TutorRowName = FName(*FString::FromInt(DayNum));
        if (FTutorialNewspaperData* TutorData = TutorialNewspaperTable->FindRow<FTutorialNewspaperData>(TutorRowName, TEXT(""))) {
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

void UIngredientFunctionLibary::SortIngredientsByPriority(const UObject* WorldContextObject, TArray<FName>& IngredientNames, bool bDescending)
{
    UCacheSubsystem* Cache = GetCacheSystem(WorldContextObject);
    if (!Cache) return;

    IngredientNames.Sort([Cache, bDescending](const FName& A, const FName& B)
        {
            const FIngredient* IngredientA = Cache->GetIngredientByRowName(A);
            const FIngredient* IngredientB = Cache->GetIngredientByRowName(B);

            int32 PriorityA = IngredientA ? IngredientA->TermsOfUse.AddPriority : TNumericLimits<int32>::Min();
            int32 PriorityB = IngredientB ? IngredientB->TermsOfUse.AddPriority : TNumericLimits<int32>::Min();

            return bDescending ? (PriorityA > PriorityB) : (PriorityA < PriorityB);
        });
}

void UIngredientFunctionLibary::GetIngredientRawVariants(const UObject* WorldContextObject, const TArray<FName>& Ingredients, TArray<FName>& RawIngredients)
{
    RawIngredients.Empty();

    TMap<FName, FName> recipeMap;
    const TArray<UConverter*> converters = GetAllConverters(WorldContextObject);
    for (const UConverter* converter : converters)
    {
        if (!converter) continue;
        for (const FConverterRecipe& recipe : converter->RecipeArray)
        {
            recipeMap.Add(recipe.To.RowName, recipe.From.RowName);
        }
    }

    TSet<FName> resultSet;
    TSet<FName> visited;

    TFunction<void(FName)> Traverse = [&](FName Ingredient)
    {
        if (visited.Contains(Ingredient)) return;
        visited.Add(Ingredient);

        if (!recipeMap.Contains(Ingredient))
        {
            resultSet.Add(Ingredient);
            return;
        }

        Traverse(recipeMap[Ingredient]);
    };

    for (const FName& ingredient : Ingredients)
    {
        Traverse(ingredient);
    }

    RawIngredients.Reserve(resultSet.Num());
    for (const FName& Raw : resultSet)
    {
        RawIngredients.Add(Raw);
    }
}

void UIngredientFunctionLibary::GetNewIngredientsOfToday(const UObject* WorldContextObject, const UGameLoop* GameLoop, const TArray<FName>& TodayIngredients, TArray<FName>& NewIngredients)
{
    NewIngredients.Empty();
    
    TArray<FName> yesterdayIngredients;
    FGameMetrics yesterdayMetrics = GameLoop->GetPreviousDayMetrics();
    GetBasePotions(WorldContextObject, yesterdayMetrics.MaxClientSymptomCount, yesterdayMetrics.HasDemonPrevious, yesterdayIngredients);

    TArray<FName> yesterdaySymptoms = GameLoop->GetPreviousDaySymptoms();
    TArray<FName> yesterdaySymptomsIngredients;
    GetIngredientsBySymptoms(WorldContextObject, yesterdaySymptoms, yesterdaySymptomsIngredients);

    yesterdayIngredients.Append(yesterdaySymptomsIngredients);

    for (const auto& ingredient : TodayIngredients) {
        if (!yesterdayIngredients.Contains(ingredient)) {
            NewIngredients.Add(ingredient);
        }
    }
}