#include "ClientsGenerator.h"
#include "SymptomStructures.h"
#include "IngredientFunctionLibary.h"
#include "Algo/RandomShuffle.h"

namespace
{
    struct FSymptomCandidate
    {
        FName Name;
        float Weight;
    };
}

ClientsGenerator::ClientsGenerator(const UObject* WorldContextObject, 
                                   const UGameProjectSettings* ProjectSettings,
                                   const TObjectPtr<UGameSettings> GameSettings,
                                   const FDaySnapshot& CurrentSnapshot,
                                   const FGameMetrics& GameMetrics)
{
    worldContextObject = WorldContextObject;
    daySnapshot = CurrentSnapshot;
    gameMetrics = GameMetrics;
    projectSettings = ProjectSettings;
    gameSettings = GameSettings.Get();

    infectionRateNormalized = (daySnapshot.VillageInfectionRate + 100.f) / 200.f;
    progressionMultiplier = 1 / (1 + FMath::Exp(-1 * gameSettings->Steepness * (gameMetrics.DayNumber - gameSettings->Midpoint)));
}

void ClientsGenerator::Process(FDaySnapshot& OutSnapshot, FGameMetrics& OutMetrics)
{
    if (!TryHandleTutorialDay())
    {
        UpdateSymptomPool();
        InitializeClients();
        FillSymptoms();
    }

    gameMetrics.SymptomMetrics.GenerateKeyArray(daySnapshot.DaySymptoms);
    gameMetrics.HasDemonPrevious = gameMetrics.HasDemonPrevious || (demonsNum > 0);

    OutSnapshot = daySnapshot;
    OutMetrics = gameMetrics;

#if !UE_BUILD_SHIPPING
    UE_LOG(LogTemp, Warning, TEXT("========================================"));
    UE_LOG(LogTemp, Warning, TEXT("[ClientsGenerator] Day %d: %d clients, %d demon(s)"),
        gameMetrics.DayNumber, daySnapshot.DayClients.Num(), demonsNum);

    UE_LOG(LogTemp, Warning, TEXT("--- Symptom pool weights ---"));
    for (const auto& SymptomPair : gameMetrics.SymptomMetrics)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s | Weight=%.2f DemonWeight=%.2f BodyPart=%d"),
            *SymptomPair.Key.ToString(),
            SymptomPair.Value.Weight,
            SymptomPair.Value.DemonWeight,
            static_cast<int32>(SymptomPair.Value.BodyPart));
    }

    UE_LOG(LogTemp, Warning, TEXT("--- Clients ---"));
    for (int32 i = 0; i < daySnapshot.DayClients.Num(); ++i)
    {
        const FClient& client = daySnapshot.DayClients[i];

        UE_LOG(LogTemp, Warning, TEXT("  Client %d [%s]:"),
            i,
            client.IsDemon ? TEXT("DEMON") : TEXT("human"));

        for (const FName& Symptom : client.Symptoms)
        {
            float symptomWeight = 0.f;
            if (const FSymptomWithWeightsData* Data = gameMetrics.SymptomMetrics.Find(Symptom))
            {
                symptomWeight = client.IsDemon ? Data->DemonWeight : Data->Weight;
            }

            UE_LOG(LogTemp, Warning, TEXT("    %s (weight=%.2f)"),
                *Symptom.ToString(),
                symptomWeight);
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("========================================"));
#endif
}

void ClientsGenerator::GenerateDemonSymptoms()
{
    daySnapshot.DemonSymptoms.Empty();

    TSet<EBodyPart> occupiedParts;
    occupiedParts.Reserve(static_cast<int32>(EBodyPart::MAX));

    for (int i = 0; i < demonSymptomCount; ++i)
    {
        FName chosenSymptom = SelectSymptomFromPool(occupiedParts, true);

        if (chosenSymptom.IsNone()) { break; }

        daySnapshot.DemonSymptoms.Add(chosenSymptom);
        occupiedParts.Add(gameMetrics.SymptomMetrics[chosenSymptom].BodyPart);

        if (FSymptomWithWeightsData* Data = gameMetrics.SymptomMetrics.Find(chosenSymptom))
        {
            Data->DemonWeight = gameSettings->WeightMinValue;
        }
    }

    float recoveryRate = gameSettings->WeightRecoveryRate;

    for (auto& unlockedSymptom : gameMetrics.SymptomMetrics)
    {
        float& currentWeight = unlockedSymptom.Value.DemonWeight;
        currentWeight = currentWeight + recoveryRate;
    }
}

void ClientsGenerator::UpdateSymptomPool()
{
    int newSymptomsNum = FMath::RandRange(gameSettings->MinNewSymptoms, gameSettings->MaxNewSymptoms);

    TMap<FName, FSymptomRow> allSymptoms;
    UIngredientFunctionLibary::GetAllSymptomsWithRowNames(worldContextObject, allSymptoms);

    TArray<FName> unlockedSymptomNames;
    gameMetrics.SymptomMetrics.GetKeys(unlockedSymptomNames);

    TArray<TPair<FName, FSymptomRow>> lockedSymptoms;
    for (const auto& pair : allSymptoms)
    {
        const FName& symptomRowName = pair.Key;
        if (!unlockedSymptomNames.Contains(symptomRowName))
        {
            lockedSymptoms.Add(TPair<FName, FSymptomRow>(symptomRowName, pair.Value));
        }
    }

    TMap<EBodyPart, int> partLockedSymptoms;
    for (const auto& symptomPair : lockedSymptoms)
    {
        int* countPtr = partLockedSymptoms.Find(symptomPair.Value.Type);
        if (countPtr)
        {
            (*countPtr)++;
        }
        else
        {
            partLockedSymptoms.Add(symptomPair.Value.Type, 1);
        }
    }

    TArray<FName> unlockedIngredientsArr;
    UIngredientFunctionLibary::GetIngredientsBySymptoms(worldContextObject, unlockedSymptomNames, unlockedIngredientsArr);
    TSet<FName> availableIngredients(unlockedIngredientsArr);

    TArray<FName> candidateIngredientNames;
    for (const auto& symptomPair : lockedSymptoms)
    {
        candidateIngredientNames.AddUnique(symptomPair.Value.IngredientRow.RowName);
    }

    TArray<UConverter*> converters = UIngredientFunctionLibary::GetAllConverters(worldContextObject);

    TMap<FName, FName> recipeMap;
    for (const UConverter* converter : converters)
    {
        if (!converter) continue;
        for (const FConverterRecipe& recipe : converter->RecipeArray)
        {
            recipeMap.Add(recipe.To.RowName, recipe.From.RowName);
            candidateIngredientNames.AddUnique(recipe.From.RowName);
            candidateIngredientNames.AddUnique(recipe.To.RowName);
        }
    }

    TArray<FName> baseIngredients;
    UIngredientFunctionLibary::GetDefaultIngredients(worldContextObject, candidateIngredientNames, baseIngredients);
    availableIngredients.Append(baseIngredients);

    TArray<TPair<FName, FSymptomRow>> remainingLocked = lockedSymptoms;
    TArray<TPair<FName, FSymptomRow>> availableSymptoms;

    bool bChanged = true;
    while (bChanged)
    {
        bChanged = false;
        for (int32 i = remainingLocked.Num() - 1; i >= 0; --i)
        {
            const FSymptomRow& symptom = remainingLocked[i].Value;
            const FName targetIngredient = symptom.IngredientRow.RowName;

            bool canOpen = availableIngredients.Contains(targetIngredient);

            if (!canOpen)
            {
                if (const FName* fromIngredient = recipeMap.Find(targetIngredient))
                {
                    canOpen = availableIngredients.Contains(*fromIngredient);
                }
            }

            if (canOpen)
            {
                availableSymptoms.Add(remainingLocked[i]);
                availableIngredients.Add(targetIngredient);
                remainingLocked.RemoveAt(i);
                bChanged = true;
            }
        }
    }

    if (availableSymptoms.IsEmpty()) return;

    availableSymptoms.Sort([&partLockedSymptoms](const TPair<FName, FSymptomRow>& A, const TPair<FName, FSymptomRow>& B) {
        int CountA = partLockedSymptoms.FindRef(A.Value.Type);
        int CountB = partLockedSymptoms.FindRef(B.Value.Type);
        if (CountA != CountB) {
            return CountA > CountB;
        }
        return FMath::RandBool();
        });

    int numToAdd = FMath::Min(newSymptomsNum, availableSymptoms.Num());
    for (int i = 0; i < numToAdd; ++i)
    {
        FSymptomWithWeightsData newSymptom;
        newSymptom.BodyPart = availableSymptoms[i].Value.Type;
        newSymptom.Weight = gameSettings->NewSymptomWeight;
        newSymptom.DemonWeight = gameSettings->NewDemonSymptomWeight;

        gameMetrics.SymptomMetrics.Add(availableSymptoms[i].Key, newSymptom);
    }
}

void ClientsGenerator::InitializeClients()
{
    float quantityMultiplier = FMath::Max(infectionRateNormalized, progressionMultiplier);
    int clientsNum = FMath::RoundToInt(gameSettings->MinClients + (gameSettings->MaxClients - gameSettings->MinClients) * quantityMultiplier);

    int32 maxDemonsClamped = FMath::Min(gameSettings->MaxDemons, clientsNum);
    int32 minDemonsClamped = FMath::Min(gameSettings->MinDemons, maxDemonsClamped);

    int32 Count = minDemonsClamped;
    while (Count < maxDemonsClamped && FMath::FRand() < gameSettings->DemonContinueChance)
    {
        ++Count;
    }
    demonsNum = Count;

    int peopleNum = clientsNum - demonsNum;

    const float symptomInfluence = FMath::Clamp(gameSettings->SymptomInfectionInfluence, 0.0f, 1.0f);
    const float smoothedInfectionRate = 0.5f + (infectionRateNormalized - 0.5f) * symptomInfluence;
    float expectedValue = gameSettings->MinSymptoms + (gameSettings->MaxSymptoms - gameSettings->MinSymptoms) * (1 - smoothedInfectionRate);

    TSet<EBodyPart> distinctUnlockedParts;
    for (const auto& pair : gameMetrics.SymptomMetrics)
    {
        distinctUnlockedParts.Add(pair.Value.BodyPart);
    }
    int32 maxAvailableSymptoms = distinctUnlockedParts.Num();

    int32 clampedMaxSymptoms = FMath::Min(gameSettings->MaxSymptoms, maxAvailableSymptoms);
    int32 clampedMinSymptoms = FMath::Min(gameSettings->MinSymptoms, clampedMaxSymptoms);

    daySnapshot.DayClients.Empty(clientsNum);
    for (int i = 0; i < peopleNum; ++i)
    {
        FClient client;
        client.IsDemon = false;
        int symptomCount = FMath::Clamp(FMath::RoundToInt(expectedValue + FMath::RandRange(-gameSettings->Error, gameSettings->Error)), clampedMinSymptoms, clampedMaxSymptoms);
        client.Symptoms.SetNum(symptomCount);
        daySnapshot.DayClients.Add(client);
    }

    demonSymptomCount = FMath::Clamp(FMath::RoundToInt(expectedValue + FMath::RandRange(-gameSettings->Error, gameSettings->Error)), clampedMinSymptoms, clampedMaxSymptoms);
    for (int i = 0; i < demonsNum; ++i)
    {
        FClient demon;
        demon.IsDemon = true;
        demon.Symptoms.SetNum(demonSymptomCount);
        daySnapshot.DayClients.Add(demon);
    }

    Algo::RandomShuffle(daySnapshot.DayClients);

#if !UE_BUILD_SHIPPING
    if (maxAvailableSymptoms < gameSettings->MinSymptoms)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ClientsGenerator] День %d: доступно только %d уникальных BodyPart, а MinSymptoms=%d. Баланс новых симптомов не поспевает за сложностью."), gameMetrics.DayNumber, maxAvailableSymptoms, gameSettings->MinSymptoms);
    }
#endif
}

bool ClientsGenerator::TryHandleTutorialDay()
{
    if (!projectSettings) { return false; }

    TObjectPtr<UDataTable> tutorialDaysTable = projectSettings->TutorialDaysTable.LoadSynchronous();
    if (!tutorialDaysTable) { return false; }

    int tutorialDayCount = tutorialDaysTable->GetRowMap().Num();
    if (gameMetrics.DayNumber > tutorialDayCount) { return false; }
    
    FName rowName = FName(FString::FromInt(gameMetrics.DayNumber));
    FTutorialDay* tutorialDayData = tutorialDaysTable->FindRow<FTutorialDay>(rowName, TEXT(""));
    if (!tutorialDayData) { return false; }

    daySnapshot.DayClients = tutorialDayData->Clients;
    demonsNum = 0;

    TSet<FName> demonSymptomsSet;
    for (auto const& client : daySnapshot.DayClients)
    {
        gameMetrics.MaxClientSymptomCount = FMath::Max(gameMetrics.MaxClientSymptomCount, client.Symptoms.Num());

        if (client.IsDemon)
        {
            demonsNum++;
        }

        for (auto const& symptom : client.Symptoms)
        {
            bool bFound = false;
            const FSymptomRow& symptomRow = UIngredientFunctionLibary::GetSymptomByRowName(worldContextObject, symptom, bFound);
            if (!bFound) 
            {
                UE_LOG(LogTemp, Error, TEXT("ClientGenerator: symptom not found - skip"));
                continue;
            }

            FSymptomWithWeightsData symptomData;
            symptomData.Weight = 1.0f;
            symptomData.DemonWeight = 1.0f;
            symptomData.BodyPart = symptomRow.Type;
            gameMetrics.SymptomMetrics.Add(symptom, symptomData);

            if (client.IsDemon)
            {
                demonSymptomsSet.Add(symptom);
            }
        }
    }

    daySnapshot.DemonSymptoms = demonSymptomsSet.Array();

    return true;
}

FName ClientsGenerator::SelectSymptomFromPool(const TSet<EBodyPart>& occupiedParts, bool isDemon)
{
    TArray<FSymptomCandidate> symptomCandidates;
    float totalWeight = 0.0f;

    for (const auto& unlockedSymptom : gameMetrics.SymptomMetrics)
    {
        const auto& symptomData = unlockedSymptom.Value;
        if (occupiedParts.Contains(symptomData.BodyPart))
        {
            continue;
        }

        float currentWeight = isDemon ? symptomData.DemonWeight : symptomData.Weight;
        if (currentWeight <= 0.0f)
        {
            continue;
        }

        symptomCandidates.Emplace(FSymptomCandidate{ unlockedSymptom.Key, currentWeight });
        totalWeight += currentWeight;
    }

    if (symptomCandidates.Num() == 0 || totalWeight <= 0.0f)
    {
        return NAME_None;
    }

    float roll = FMath::RandRange(0.0f, totalWeight);
    float accumulatedWeight = 0.0f;

    for (const auto& candidate : symptomCandidates)
    {
        accumulatedWeight += candidate.Weight;
        if (roll <= accumulatedWeight)
        {
            return candidate.Name;
        }
    }

    return NAME_None;
}

void ClientsGenerator::FillSymptoms()
{
    GenerateDemonSymptoms();

    // demonSymptomCount мог быть посчитан оптимистично,
    // а GenerateDemonSymptoms мог реально набрать меньше — синхронизируем.
    demonSymptomCount = daySnapshot.DemonSymptoms.Num();

    float recoveryRate = gameSettings->WeightRecoveryRate;

    for (int i = 0; i < daySnapshot.DayClients.Num(); ++i)
    {
        if (daySnapshot.DayClients[i].IsDemon)
        {
            // Обрезаем массив демона под реально собранный список DemonSymptoms —
            // никаких "хвостов" из None.
            daySnapshot.DayClients[i].Symptoms.SetNum(demonSymptomCount);
            for (int j = 0; j < demonSymptomCount; ++j)
            {
                daySnapshot.DayClients[i].Symptoms[j] = daySnapshot.DemonSymptoms[j];
            }
        }
        else
        {
            GenerateSymptomsForClient(daySnapshot.DayClients[i]);

            for (auto& unlockedSymptom : gameMetrics.SymptomMetrics)
            {
                float& currentWeight = unlockedSymptom.Value.Weight;
                currentWeight = currentWeight + recoveryRate;
            }
        }

        gameMetrics.MaxClientSymptomCount = FMath::Max(gameMetrics.MaxClientSymptomCount, daySnapshot.DayClients[i].Symptoms.Num());
    }
}

void ClientsGenerator::GenerateSymptomsForClient(FClient& client)
{
    TSet<EBodyPart> occupiedParts;
    occupiedParts.Reserve(static_cast<int32>(EBodyPart::MAX));

    int32 requestedCount = client.Symptoms.Num();
    int32 filledCount = 0;

    for (int i = 0; i < requestedCount; ++i)
    {
        FName chosenSymptom = SelectSymptomFromPool(occupiedParts, false);

        if (chosenSymptom.IsNone())
        {
#if !UE_BUILD_SHIPPING
            UE_LOG(LogTemp, Warning,
                TEXT("[ClientsGenerator] GenerateSymptomsForClient: pool exhausted at %d/%d, occupiedParts=%d, unlockedSymptoms=%d"),
                filledCount, requestedCount, occupiedParts.Num(), gameMetrics.SymptomMetrics.Num());
#endif
            break;
        }

        occupiedParts.Add(gameMetrics.SymptomMetrics[chosenSymptom].BodyPart);
        client.Symptoms[filledCount] = chosenSymptom;
        ++filledCount;

        if (FSymptomWithWeightsData* Data = gameMetrics.SymptomMetrics.Find(chosenSymptom))
        {
            Data->Weight = gameSettings->WeightMinValue;
        }
    }

    client.Symptoms.SetNum(filledCount);

    EnsureSymptomsDontMatchDemon(client);
}

void ClientsGenerator::EnsureSymptomsDontMatchDemon(FClient& client)
{
    if (client.Symptoms.Num() == 0 || client.Symptoms.Num() != daySnapshot.DemonSymptoms.Num())
    {
        return; // разное количество симптомов — совпасть как множество не может
    }

    TSet<FName> clientSet(client.Symptoms);
    TSet<FName> demonSet(daySnapshot.DemonSymptoms);

    if (clientSet.Num() != demonSet.Num() || clientSet.Difference(demonSet).Num() != 0)
    {
        return; // не совпадает — всё ок
    }

    // Полное совпадение — подменяем один случайный симптом клиента на что-то другое.
    TSet<EBodyPart> occupiedParts;
    for (const FName& Symptom : client.Symptoms)
    {
        if (const FSymptomWithWeightsData* Data = gameMetrics.SymptomMetrics.Find(Symptom))
        {
            occupiedParts.Add(Data->BodyPart);
        }
    }

    int32 IndexToReplace = FMath::RandRange(0, client.Symptoms.Num() - 1);
    FName OldSymptom = client.Symptoms[IndexToReplace];

    if (const FSymptomWithWeightsData* OldData = gameMetrics.SymptomMetrics.Find(OldSymptom))
    {
        occupiedParts.Remove(OldData->BodyPart); // освобождаем часть тела, чтобы можно было выбрать замену
    }

    FName NewSymptom = SelectSymptomFromPool(occupiedParts, false);

    if (!NewSymptom.IsNone() && NewSymptom != OldSymptom)
    {
        client.Symptoms[IndexToReplace] = NewSymptom;

        if (FSymptomWithWeightsData* NewData = gameMetrics.SymptomMetrics.Find(NewSymptom))
        {
            NewData->Weight = gameSettings->WeightMinValue;
        }
    }

#if !UE_BUILD_SHIPPING
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[ClientsGenerator] EnsureSymptomsDontMatchDemon: could not find a replacement symptom, client may still match the demon"));
    }
#endif
}
