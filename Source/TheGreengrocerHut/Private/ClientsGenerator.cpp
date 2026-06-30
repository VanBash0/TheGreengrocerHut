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
}

void ClientsGenerator::GenerateDemonSymptoms()
{
    daySnapshot.DemonSymptoms.Empty();
    TSet<EBodyPart> occupiedParts;
    occupiedParts.Reserve(static_cast<int32>(EBodyPart::MAX));

    for (int i = 0; i < demonSymptomCount; ++i) {
        FName chosenSymptom = SelectSymptomFromPool(occupiedParts, true);

        if (chosenSymptom.IsNone()) { break; }

        daySnapshot.DemonSymptoms.Add(chosenSymptom);
        occupiedParts.Add(gameMetrics.SymptomMetrics[chosenSymptom].BodyPart);

        if (FSymptomWithWeightsData* Data = gameMetrics.SymptomMetrics.Find(chosenSymptom)) { Data->DemonWeight = 0.0f; }
    }

    float recoveryRate = gameSettings->WeightRecoveryRate;
    for (auto& unlockedSymptom : gameMetrics.SymptomMetrics)
    {
        float& currentWeight = unlockedSymptom.Value.DemonWeight;
        if (currentWeight > 0.0f)
        {
            currentWeight = FMath::Min(currentWeight + recoveryRate, gameSettings->WeightMinValue);
        }
    }
}

void ClientsGenerator::UpdateSymptomPool()
{
    int newSymptomsNum = FMath::RandRange(gameSettings->MinNewSymptoms, gameSettings->MaxNewSymptoms);

    TArray<FSymptomRow> allSymptoms = UIngredientFunctionLibary::GetAllSymptoms(worldContextObject);
    TArray<FName> unlockedSymptomNames;
    for (const auto& symptom : gameMetrics.SymptomMetrics) {
        unlockedSymptomNames.Add(symptom.Key);
    }

    TArray<FSymptomRow> lockedSymptoms;
    for (const FSymptomRow& symptom : allSymptoms)
    {
        FName symptomName = FName(*symptom.Name.ToString());
        if (!unlockedSymptomNames.Contains(symptomName))
        {
            lockedSymptoms.Add(symptom);
        }
    }

    TMap<EBodyPart, int> partLockedSymptoms;
    for (const FSymptomRow& symptom : lockedSymptoms)
    {
        int* countPtr = partLockedSymptoms.Find(symptom.Type);
        if (countPtr) (*countPtr)++;
        else partLockedSymptoms.Add(symptom.Type, 1);
    }

    TArray<FName> unlockedIngredients;
    UIngredientFunctionLibary::GetIngredientsBySymptoms(worldContextObject, unlockedSymptomNames, unlockedIngredients);
    TArray<UConverter*> converters = UIngredientFunctionLibary::GetAllConverters(worldContextObject);

    TArray<FSymptomRow> availableSymptoms;
    for (const FSymptomRow& symptom : lockedSymptoms) {
        bool canOpen = false;
        for (UConverter* converter : converters) {
            for (const FConverterRecipe& recipe : converter->RecipeArray) {
                if (recipe.To == symptom.IngredientRow) {                    
                    if (unlockedIngredients.Contains(recipe.From.RowName)) {
                        canOpen = true;
                        break;
                    }
                }
            }
            if (canOpen) break;
        }
        if (canOpen) {
            availableSymptoms.Add(symptom);
        }
    }

    if (availableSymptoms.Num() == 0) return;

    availableSymptoms.Sort([&partLockedSymptoms](const FSymptomRow& A, const FSymptomRow& B) {
        int CountA = partLockedSymptoms.FindRef(A.Type);
        int CountB = partLockedSymptoms.FindRef(B.Type);
        if (CountA != CountB) {
            return CountA > CountB;
        }
        return FMath::RandBool();
    });

    int numToAdd = FMath::Min(newSymptomsNum, availableSymptoms.Num());
    for (int i = 0; i < numToAdd; ++i) {
        FSymptomWithWeightsData newSymptom;
        newSymptom.BodyPart = availableSymptoms[i].Type;
        newSymptom.Weight = gameSettings->NewSymptomWeight;
        newSymptom.DemonWeight = gameSettings->NewDemonSymptomWeight;
        FName symptomName = FName(availableSymptoms[i].Name.ToString());
        gameMetrics.SymptomMetrics.Add(symptomName, newSymptom);
    }
}

void ClientsGenerator::InitializeClients()
{
    float quantityMultiplier = FMath::Max(infectionRateNormalized, progressionMultiplier);
    int clientsNum = FMath::RoundToInt(gameSettings->MinClients + (gameSettings->MaxClients - gameSettings->MinClients) 
        * quantityMultiplier);
    demonsNum = FMath::RandRange(0, FMath::Min(gameSettings->MaxDemons, clientsNum));
    float expectedValue = gameSettings->MinSymptoms + (gameSettings->MaxSymptoms - gameSettings->MinSymptoms)
        * (1 - infectionRateNormalized);

    int peopleNum = clientsNum - demonsNum;
    daySnapshot.DayClients = TArray<FClient>();
    for (int i = 0; i < peopleNum; ++i) {
        FClient client;
        client.IsDemon = false;
        int symptomCount = FMath::Clamp(FMath::RoundToInt(expectedValue + FMath::RandRange(-gameSettings->Error, gameSettings->Error)),
            gameSettings->MinSymptoms, gameSettings->MaxSymptoms);
        client.Symptoms.SetNum(symptomCount);
        daySnapshot.DayClients.Add(client);
    }

    demonSymptomCount = FMath::Clamp(FMath::RoundToInt(expectedValue + FMath::RandRange(-gameSettings->Error, gameSettings->Error)),
        gameSettings->MinSymptoms, gameSettings->MaxSymptoms);
    for (int i = 0; i < demonsNum; ++i) {
        FClient demon;
        demon.IsDemon = true;
        demon.Symptoms.SetNum(demonSymptomCount);
        daySnapshot.DayClients.Add(demon);
    }

    Algo::RandomShuffle(daySnapshot.DayClients);
}

bool ClientsGenerator::TryHandleTutorialDay()
{
    if (!projectSettings) { return false; }
    TObjectPtr<UDataTable> tutorialDaysTable = projectSettings->TutorialDaysTable.LoadSynchronous();
    if (!tutorialDaysTable) { return false; }

    int tutorialDayCount = tutorialDaysTable->GetRowMap().Num();
    if (gameMetrics.DayNumber <= tutorialDayCount) {
        FName rowName = FName(FString::FromInt(gameMetrics.DayNumber));
        FTutorialDay* tutorialDayData = tutorialDaysTable->FindRow<FTutorialDay>(rowName, TEXT(""));

        if (!tutorialDayData) { return false; }

        daySnapshot.DayClients = tutorialDayData->Clients;
        TSet<FName> demonSymptomsSet;
        for (auto const& client : daySnapshot.DayClients) {
            gameMetrics.MaxClientSymptomCount = FMath::Max(gameMetrics.MaxClientSymptomCount, client.Symptoms.Num());
            for (auto const& symptom : client.Symptoms) {
                if (!tutorialDaysTable) continue;

                bool bFound = false;
                const FSymptomRow& symptomRow = UIngredientFunctionLibary::GetSymptomByRowName(worldContextObject, symptom, bFound);
                if (!bFound) { continue; }

                FSymptomWithWeightsData symptomData;
                symptomData.Weight = 1.0f;
                symptomData.DemonWeight = 1.0f;
                symptomData.BodyPart = symptomRow.Type;
                gameMetrics.SymptomMetrics.Add(symptom, symptomData);

                if (client.IsDemon) {
                    demonSymptomsSet.Add(symptom);
                }
            }
        }
        daySnapshot.DemonSymptoms = demonSymptomsSet.Array();

        return true;
    }

    return false;
}

FName ClientsGenerator::SelectSymptomFromPool(const TSet<EBodyPart>& occupiedParts, bool isDemon)
{
    TArray<FSymptomCandidate> symptomCandidates;
    float totalWeight = 0.0f;

    for (const auto& unlockedSymptom : gameMetrics.SymptomMetrics) {
        const auto& symptomData = unlockedSymptom.Value;
        if (occupiedParts.Contains(symptomData.BodyPart)) {
            continue;
        }

        float currentWeight = isDemon ? symptomData.DemonWeight : symptomData.Weight;
        if (currentWeight <= 0.0f) {
            continue;
        }

        symptomCandidates.Emplace(FSymptomCandidate{ unlockedSymptom.Key, currentWeight });
        totalWeight += currentWeight;
    }

    if (symptomCandidates.Num() == 0 || totalWeight <= 0.0f) {
        return NAME_None;
    }

    float roll = FMath::RandRange(0.0f, totalWeight);
    float accumulatedWeight = 0.0f;

    for (const auto& candidate : symptomCandidates) {
        accumulatedWeight += candidate.Weight;
        if (roll <= accumulatedWeight) {
            return candidate.Name;
        }
    }

    return NAME_None;
}

void ClientsGenerator::FillSymptoms()
{
    if (demonsNum > 0) 
    {
        GenerateDemonSymptoms();
    }

    for (int i = 0; i < daySnapshot.DayClients.Num(); ++i)
    {
        gameMetrics.MaxClientSymptomCount = FMath::Max(gameMetrics.MaxClientSymptomCount, daySnapshot.DayClients[i].Symptoms.Num());
        if (daySnapshot.DayClients[i].IsDemon)
        {
            for (int j = 0; j < demonSymptomCount; ++j)
            {
                if (daySnapshot.DemonSymptoms.IsValidIndex(j))
                {
                    daySnapshot.DayClients[i].Symptoms[j] = daySnapshot.DemonSymptoms[j];
                }
            }
        }
        else
        {
            GenerateSymptomsForClient(daySnapshot.DayClients[i]);
        }
    }
}

void ClientsGenerator::GenerateSymptomsForClient(FClient& client) {
    TSet<EBodyPart> occupiedParts;
    occupiedParts.Reserve(static_cast<int32>(EBodyPart::MAX));

    for (int i = 0; i < client.Symptoms.Num(); ++i) {
        FName chosenSymptom = SelectSymptomFromPool(occupiedParts, false);

        if (chosenSymptom.IsNone()) {
            break;
        }

        occupiedParts.Add(gameMetrics.SymptomMetrics[chosenSymptom].BodyPart);
        client.Symptoms[i] = chosenSymptom;

        if (FSymptomWithWeightsData* Data = gameMetrics.SymptomMetrics.Find(chosenSymptom)) { Data->Weight = 0.0f; }
    }

    float recoveryRate = gameSettings->WeightRecoveryRate;
    for (auto& unlockedSymptom : gameMetrics.SymptomMetrics)
    {
        float& currentWeight = unlockedSymptom.Value.Weight;
        if (currentWeight > 0.0f) {
            currentWeight = FMath::Min(currentWeight + recoveryRate, gameSettings->WeightMinValue);
        }
    }
}