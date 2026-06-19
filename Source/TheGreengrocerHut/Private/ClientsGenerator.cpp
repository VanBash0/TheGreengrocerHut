#include "ClientsGenerator.h"
#include "SymptomStructures.h"

namespace
{
    struct FSymptomCandidate
    {
        FName Name;
        float Weight;
    };
}

void ClientsGenerator::InitializeVariables(const UGameProjectSettings* ProjectSettings, const TObjectPtr<UGameSettings> GameSettings,
    FClientsGeneratorData& ClientsGeneratorData)
{
    generatorData = ClientsGeneratorData;
    projectSettings = ProjectSettings;
    gameSettings = GameSettings.Get();
    infectionRateNormalized = (generatorData.InfectionRate + 100.f) / 200.f;
    progressionMultiplier = 1 / (1 + FMath::Exp(-1 * gameSettings->Steepness * (generatorData.DayNumber - gameSettings->Midpoint)));
    unlockedSymptoms = ClientsGeneratorData.UnlockedSymptoms;
}

void ClientsGenerator::UpdateSymptomPool()
{
    int NewSymptomsNum = FMath::RandRange(1, gameSettings->MaxNewSymptoms);

    // TODO: доделать обновление с учетом BPDA_Ingredient
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
    clients = TArray<FClient>();
    for (int i = 0; i < peopleNum; ++i) {
        FClient client;
        client.IsDemon = false;
        int symptomCount = FMath::Clamp(FMath::RoundToInt(expectedValue + FMath::RandRange(-gameSettings->Error, gameSettings->Error)),
            gameSettings->MinSymptoms, gameSettings->MaxSymptoms);
        client.Symptoms.SetNum(symptomCount);
        clients.Add(client);
    }

    demonSymptomCount = FMath::Clamp(FMath::RoundToInt(expectedValue + FMath::RandRange(-gameSettings->Error, gameSettings->Error)),
        gameSettings->MinSymptoms, gameSettings->MaxSymptoms);
    for (int i = 0; i < demonsNum; ++i) {
        FClient demon;
        demon.IsDemon = true;
        demon.Symptoms.SetNum(demonSymptomCount);
        clients.Add(demon);
    }
}

bool ClientsGenerator::TryHandleTutorialDay()
{
    if (!projectSettings || !projectSettings->TutorialDaysTable)
        return false;

    int tutorialDayCount = projectSettings->TutorialDaysTable->GetRowMap().Num();
    if (generatorData.DayNumber <= tutorialDayCount) {
        FName rowName = FName(FString::FromInt(generatorData.DayNumber));
        FTutorialDay* tutorialDayData = projectSettings->TutorialDaysTable->FindRow<FTutorialDay>(rowName, TEXT(""));

        if (!tutorialDayData) return false;

        clients = tutorialDayData->Clients;
        unlockedSymptoms.Empty();
        for (auto const& client : clients) {
            for (auto const& symptom : client.Symptoms) {
                if (!projectSettings->SymptomTable) continue;
                FSymptomRow* symptomRow = projectSettings->SymptomTable->FindRow<FSymptomRow>(symptom, TEXT(""));
                if (!symptomRow) continue;

                FSymptomWithWeightsData symptomData;
                symptomData.Weight = 1.0f;
                symptomData.DemonWeight = 1.0f;
                symptomData.BodyPart = symptomRow->Type;
            }
        }
        return true;
    }
    return false;
}

FName ClientsGenerator::SelectSymptomFromPool(const TSet<EBodyPart>& occupiedParts, bool isDemon)
{
    TArray<FSymptomCandidate> symptomCandidates;
    float totalWeight = 0.0f;

    for (const auto& unlockedSymptom : unlockedSymptoms) {
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
    if (demonsNum > 0) {
        GenerateDemonSymptoms(demonSymptomCount);
    }

    int clientsNum = clients.Num();
    for (int i = 0; i < clientsNum; ++i) {
        if (clients[i].IsDemon) {
            for (int j = 0; j < demonSymptomCount; ++j) {
                if (demonSymptomsOfDay.IsValidIndex(j)) {
                    clients[i].Symptoms[j] = demonSymptomsOfDay[j];
                }
            }
        }
        else {
            GenerateSymptomsForClient(clients[i]);
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

        occupiedParts.Add(unlockedSymptoms[chosenSymptom].BodyPart);
        client.Symptoms[i] = chosenSymptom;

        unlockedSymptoms[chosenSymptom].Weight = 0.0f;
    }

    float recoveryRate = gameSettings->WeightRecoveryRate;
    for (auto& unlockedSymptom : unlockedSymptoms)
    {
        float& currentWeight = unlockedSymptom.Value.Weight;
        if (currentWeight > 0.0f) {
            currentWeight += recoveryRate;
        }
    }
}

void ClientsGenerator::GenerateDemonSymptoms(int symptomCount)
{
    demonSymptomsOfDay.Empty();
    TSet<EBodyPart> occupiedParts;
    occupiedParts.Reserve(static_cast<int32>(EBodyPart::MAX));

    for (int i = 0; i < symptomCount; ++i) {
        FName chosenSymptom = SelectSymptomFromPool(occupiedParts, true);

        if (chosenSymptom.IsNone()) {
            break;
        }

        demonSymptomsOfDay.Add(chosenSymptom);
        occupiedParts.Add(unlockedSymptoms[chosenSymptom].BodyPart);

        unlockedSymptoms[chosenSymptom].DemonWeight = 0.0f;
    }

    float recoveryRate = gameSettings->WeightRecoveryRate;
    for (auto& unlockedSymptom : unlockedSymptoms)
    {
        float& currentWeight = unlockedSymptom.Value.DemonWeight;
        if (currentWeight > 0.0f) {
            currentWeight += recoveryRate;
        }
    }
}

TArray<FClient> ClientsGenerator::GenerateClients(const UGameProjectSettings* ProjectSettings, const TObjectPtr<UGameSettings> GameSettings,
    FClientsGeneratorData& ClientsGeneratorData)
{
    InitializeVariables(ProjectSettings, GameSettings, ClientsGeneratorData);

    if (TryHandleTutorialDay()) {
        return clients;
    }

    UpdateSymptomPool();
    InitializeClients();
    FillSymptoms();

    return clients;
}