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

void UIngredientFunctionLibary::GetBasePotions(const UObject* WorldContextObject, const int& MaxClientSymptomCount, const bool& HasDemonPrevious, TArray<FName>& BasePotions)
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

void UIngredientFunctionLibary::GetBasePotionBySumptomCount(const UObject* WorldContextObject, const UGameSettings* GameSettings, const int& ClientSymptomCount, FName& Potion)
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

// ===== ИНТЕРПРЕТАЦИЯ РЕЗУЛЬТАТА КЛИЕНТА =====
void UIngredientFunctionLibary::CalculateClientResult(const UObject* WorldContextObject, const TArray<FIngredient>& Ingredients, const UGameLoop* GameLoop, FClientResult& OutResult)
{
    OutResult = FClientResult();
    GameLoop->GetCurrentClient(OutResult.Client);
    const FClient& currentClient = OutResult.Client;

    FPotionCompareResult Compare;
    ComparePotionToRecipe(WorldContextObject, Ingredients, GameLoop, Compare);

    OutResult.GivenIngredients = Compare.GivenIngredients;
    OutResult.IngredientValidity = Compare.IngredientValidity;
    OutResult.PotionMatchScore = Compare.PotionMatchScore;
    OutResult.TierMatchResults = Compare.TierMatchResults;

    if (currentClient.IsDemon)
    {
        if (Compare.IsPoison)
        {
            OutResult.Result = EPotionResult::Demon_Poisoned;
            OutResult.IsPotionGood = true;
        }
        else if (Compare.PotionMatchScore >= 0.5f)
        {
            OutResult.Result = EPotionResult::Demon_GivenGoodPotion;
        }
        else
        {
            OutResult.Result = EPotionResult::Demon_NotPoisoned;
        }
    }
    else
    {
        if (Compare.IsPoison)
        {
            OutResult.Result = EPotionResult::Human_Poisoned;
        }
        else if (Compare.ValidFraction >= 0.5f)
        {
            OutResult.Result = EPotionResult::Human_Healed;
            OutResult.IsPotionGood = true;
        }
        else
        {
            OutResult.Result = EPotionResult::Human_NotHealed;
        }
    }

    const TObjectPtr<UGameSettings>& gameSettings = GameLoop->GameSettings;

    FGameMetrics gameMetrics;
    GameLoop->GetGameMetrics(gameMetrics);
    const float dayMultiplier = 1.f + 0.02f * (gameMetrics.DayNumber - 1);

    const float* BaseDeltaPtr = gameSettings->PotionResultDeltaMap.Find(OutResult.Result);
    float delta = BaseDeltaPtr ? *BaseDeltaPtr : 0.f;

    switch (OutResult.Result)
    {
    case EPotionResult::Human_Healed:
        delta *= dayMultiplier * gameMetrics.HealingFactor * Compare.ValidFraction;
        break;

    case EPotionResult::Human_NotHealed:
        delta *= dayMultiplier * gameMetrics.KillingFactor * (1.f - Compare.ValidFraction);
        break;

    case EPotionResult::Human_Poisoned:
    case EPotionResult::Demon_Poisoned:
        delta *= dayMultiplier * gameMetrics.HealingFactor;
        break;

    case EPotionResult::Demon_NotPoisoned:
    case EPotionResult::Demon_GivenGoodPotion:
        delta *= dayMultiplier * gameMetrics.KillingFactor;
        break;
    }

    OutResult.DeltaInfectionRate = delta;
}

namespace
{
    // Размер пересечения двух мультимножеств FName — используется ТОЛЬКО внутри
    // одного приоритетного уровня, где порядок между элементами не важен.
    int32 CountMultisetIntersection(const TArray<FName>& A, const TArray<FName>& B)
    {
        TMap<FName, int32> CountsA;
        for (const FName& Name : A) { CountsA.FindOrAdd(Name)++; }

        int32 Intersection = 0;
        for (const FName& Name : B)
        {
            int32* Remaining = CountsA.Find(Name);
            if (Remaining && *Remaining > 0)
            {
                --(*Remaining);
                ++Intersection;
            }
        }
        return Intersection;
    }

    // Локальная группа ингредиентов рецепта с одним и тем же приоритетом
    // (-1 База, 0 Основной, 1 Дополнительный, 2 Связывающий — имена/значения из GameSettings->IngredientPriorityData)
    struct FCanonicalGroup
    {
        int32 Priority = 0;
        TArray<FName> Names;
    };

    // Группирует список ингредиентов по их фактическому AddPriority (не зависит от позиции/порядка)
    TMap<int32, TArray<FName>> GroupByPriority(UCacheSubsystem* Cache, const TArray<FName>& Names)
    {
        TMap<int32, TArray<FName>> Result;
        for (const FName& Name : Names)
        {
            if (Name.IsNone()) { continue; }

            const FIngredient* IngredientData = Cache ? Cache->GetIngredientByRowName(Name) : nullptr;
            const int32 Priority = IngredientData ? IngredientData->TermsOfUse.AddPriority : 0;
            Result.FindOrAdd(Priority).Add(Name);
        }
        return Result;
    }
}

// ===== ЧИСТОЕ СРАВНЕНИЕ ЗЕЛЬЯ С РЕЦЕПТОМ =====
// Ничего не знает про демонов/людей/дельты — только "что дали" против "что нужно было дать".
void UIngredientFunctionLibary::ComparePotionToRecipe(const UObject* WorldContextObject, const TArray<FIngredient>& Ingredients, const UGameLoop* GameLoop, FPotionCompareResult& OutCompare)
{
    OutCompare = FPotionCompareResult();
    OutCompare.GivenIngredients.Reserve(Ingredients.Num());
    OutCompare.IngredientValidity.Reserve(Ingredients.Num());

    FClient currentClient;
    GameLoop->GetCurrentClient(currentClient);

    // ингредиенты по симптомам (без базы) — то, что нужно добавить сверх базы
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
    OutCompare.GivenIngredients = playerIngredientNames;

    // === Строим канонический рецепт: база + нужные по симптомам, ВСЕ вместе, ===
    // === сгруппированные по фактическому AddPriority (база сама встанет первой, у неё Priority == -1) ===
    UCacheSubsystem* Cache = GetCacheSystem(WorldContextObject);

    TArray<FName> allRecipeIngredients = neededIngredients;
    if (!requiredBase.IsNone())
    {
        allRecipeIngredients.Add(requiredBase);
    }

    allRecipeIngredients.Sort([Cache](const FName& A, const FName& B)
        {
            const FIngredient* IA = Cache ? Cache->GetIngredientByRowName(A) : nullptr;
            const FIngredient* IB = Cache ? Cache->GetIngredientByRowName(B) : nullptr;
            const int32 PA = IA ? IA->TermsOfUse.AddPriority : 0;
            const int32 PB = IB ? IB->TermsOfUse.AddPriority : 0;
            return PA < PB;
        });

    TArray<FCanonicalGroup> CanonicalGroups;
    for (const FName& Name : allRecipeIngredients)
    {
        const FIngredient* IngredientData = Cache ? Cache->GetIngredientByRowName(Name) : nullptr;
        const int32 Priority = IngredientData ? IngredientData->TermsOfUse.AddPriority : 0;

        if (CanonicalGroups.Num() == 0 || CanonicalGroups.Last().Priority != Priority)
        {
            CanonicalGroups.AddDefaulted();
            CanonicalGroups.Last().Priority = Priority;
        }
        CanonicalGroups.Last().Names.Add(Name);
    }

    // === PotionMatchScore: блочное позиционное совпадение ===
    // Идём по канонической последовательности приоритетных блоков, "откусывая" от
    // player-последовательности ровно столько ингредиентов, сколько в текущем блоке,
    // сравниваем этот кусок как мультимножество. Лишние ингредиенты сверху раздувают
    // знаменатель (max(данного, нужного)) и проседают в скор, даже если "нужная часть" верна.
    int32 TotalNeeded = 0;
    int32 PositionalMatches = 0;
    {
        int32 PlayerIndex = 0;
        for (const FCanonicalGroup& Group : CanonicalGroups)
        {
            TotalNeeded += Group.Names.Num();

            TArray<FName> PlayerSlice;
            for (int32 k = 0; k < Group.Names.Num() && PlayerIndex < playerIngredientNames.Num(); ++k, ++PlayerIndex)
            {
                PlayerSlice.Add(playerIngredientNames[PlayerIndex]);
            }

            PositionalMatches += CountMultisetIntersection(PlayerSlice, Group.Names);
        }

        const int32 Denominator = FMath::Max(playerIngredientNames.Num(), TotalNeeded);
        OutCompare.PotionMatchScore = Denominator > 0
            ? static_cast<float>(PositionalMatches) / static_cast<float>(Denominator)
            : 1.f; // оба списка пустые -> считаем точным совпадением
    }

    // === TierMatchResults: совпадение НЕЗАВИСИМО ОТ ПОЗИЦИИ, по фактическому приоритету игрока ===
    // Ключ = union приоритетов из нужного рецепта И того, что реально дал игрок.
    // Если игрок кинул лишнее (в уже занятый уровень или в вообще не нужный) — мультимножества
    // не совпадут по размеру, и уровень станет false, даже если "нужная" часть внутри присутствует.
    {
        UCacheSubsystem* CacheForGiven = Cache; // тот же кэш, что и для рецепта

        TMap<int32, TArray<FName>> NeededByPriority;
        for (const FCanonicalGroup& Group : CanonicalGroups)
        {
            NeededByPriority.Add(Group.Priority, Group.Names);
        }

        const TMap<int32, TArray<FName>> GivenByPriority = GroupByPriority(CacheForGiven, playerIngredientNames);

        TSet<int32> AllPriorities;
        NeededByPriority.GetKeys(AllPriorities);
        for (const auto& Pair : GivenByPriority) { AllPriorities.Add(Pair.Key); }

        for (int32 Priority : AllPriorities)
        {
            const TArray<FName>* NeededAtTier = NeededByPriority.Find(Priority);
            const TArray<FName>* GivenAtTier = GivenByPriority.Find(Priority);

            const int32 NeededCount = NeededAtTier ? NeededAtTier->Num() : 0;
            const int32 GivenCount = GivenAtTier ? GivenAtTier->Num() : 0;

            const int32 Intersection = (NeededAtTier && GivenAtTier)
                ? CountMultisetIntersection(*GivenAtTier, *NeededAtTier)
                : 0;

            const bool bTierMatched = (NeededCount == GivenCount) && (Intersection == NeededCount);
            OutCompare.TierMatchResults.Add(Priority, bTierMatched);
        }
    }

    const bool bExactRecipeMatch = OutCompare.PotionMatchScore >= 1.0f;

    // "база на своём месте" — отдельно от score/tiers, нужно для валидности конкретно 0-го слота и детекта яда
    const bool bBaseOk = !requiredBase.IsNone()
        && playerIngredientNames.Num() > 0
        && (playerIngredientNames[0] == requiredBase);

    // проверка порядка по приоритету (после базы — неубывающий) + подсчёт валидных
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
            }

            // Яд детектится независимо от того, была ли эта база "правильной" —
            // если игрок дал человеку яд, это яд, даже если requiredBase был другим.
            if (ingredientName == gameSettings->PoisonBase.RowName)
            {
                OutCompare.IsPoison = true;
            }
        }
        else
        {
            bIsValid = bExactRecipeMatch
                && !ingredientName.IsNone()
                && ingredient.TermsOfUse.AddPriority >= currentPriority;

            if (bIsValid)
            {
                matchingIngredients++;
                currentPriority = FMath::Max(currentPriority, ingredient.TermsOfUse.AddPriority);
                if (ingredientName == gameSettings->PoisonBase.RowName)
                {
                    OutCompare.IsPoison = true;
                }
            }
        }

        OutCompare.IngredientValidity.Add(bIsValid);
    }

    OutCompare.ValidFraction = Ingredients.Num() > 0
        ? static_cast<float>(matchingIngredients) / static_cast<float>(Ingredients.Num())
        : 0.f;
}

namespace
{
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