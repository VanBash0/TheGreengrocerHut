#include "ClientsGenerator.h"
#include "SymptomStructures.h"

void ClientsGenerator::InitializeVariables(const UGameProjectSettings& ProjectSettings, const UGameSettings& GameSettings,
    const FClientsGeneratorData& ClientsGeneratorData)
{
    generatorData = ClientsGeneratorData;
    projectSettings = &ProjectSettings;
    gameSettings = &GameSettings;
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
    int demonsNum = FMath::RandRange(0, FMath::Min(gameSettings->MaxDemons, clientsNum));
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

    int demonSymptomCount = FMath::Clamp(FMath::RoundToInt(expectedValue + FMath::RandRange(-gameSettings->Error, gameSettings->Error)),
        gameSettings->MinSymptoms, gameSettings->MaxSymptoms);
    demons = TArray<FClient>();
    for (int i = 0; i < demonsNum; ++i) {
        FClient demon;
        demon.IsDemon = true;
        demon.Symptoms.SetNum(demonSymptomCount);
        demons.Add(demon);
    }
}

void ClientsGenerator::FillSymptoms()
{
    
}

bool ClientsGenerator::TryHandleTutorialDay()
{
    int tutorialDayCount = projectSettings->TutorialDaysTable->GetRowMap().Num();
    if (generatorData.DayNumber <= tutorialDayCount) {
        FName rowName = FName(FString::FromInt(generatorData.DayNumber));
        FTutorialDay* tutorialDayData = projectSettings->TutorialDaysTable->FindRow<FTutorialDay>(rowName, TEXT(""));
        clients = tutorialDayData->Clients;
        unlockedSymptoms.Empty();
        for (auto const& client : clients) {
            auto symptoms = client.Symptoms;
            for (auto const& symptom : symptoms) {
                FSymptomWithWeightsData symptomData;
                symptomData.Weight = 1.0f;
                symptomData.DemonWeight = 1.0f;
                FSymptomRow* symptomRow = projectSettings->SymptomTable->FindRow<FSymptomRow>(symptom, TEXT(""));
                symptomData.BodyPart = symptomRow->Type;
            }
        }
        return true;
    }
    return false;
}

TArray<FClient> ClientsGenerator::GenerateClients(const UGameProjectSettings& ProjectSettings, const UGameSettings& GameSettings,
    const FClientsGeneratorData& ClientsGeneratorData)
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