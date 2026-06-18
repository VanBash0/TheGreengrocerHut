#include "ClientsGenerator.h"

void ClientsGenerator::InitializeVariables(const UGameSettings& GameSettings, const FClientsGeneratorData& ClientsGeneratorData)
{
    generatorData = ClientsGeneratorData;
    settingsData = &GameSettings;
    infectionRateNormalized = (generatorData.InfectionRate + 100.f) / 200.f;
    progressionMultiplier = 1 / (1 + FMath::Exp(-1 * settingsData->Steepness * (generatorData.DayNumber - settingsData->Midpoint)));
}

void ClientsGenerator::UpdateSymptomPool()
{
    int NewSymptomsNum = FMath::RandRange(1, settingsData->MaxNewSymptoms);

    // TODO: доделать обновление с учетом BPDA_Ingredient
}

void ClientsGenerator::InitializeClients()
{
    float quantityMultiplier = FMath::Max(infectionRateNormalized, progressionMultiplier);
    int clientsNum = FMath::RoundToInt(settingsData->MinClients + (settingsData->MaxClients - settingsData->MinClients) 
        * quantityMultiplier);
    float expectedValue = settingsData->MinSymptoms + (settingsData->MaxSymptoms - settingsData->MinSymptoms)
        * (1 - infectionRateNormalized);

    clients = TArray<FClient>();
    for (int i = 0; i < clientsNum; ++i) {
        FClient client;
        client.IsDemon = false;
        int symptomCount = FMath::Clamp(FMath::RoundToInt(expectedValue + FMath::RandRange(-settingsData->Error, settingsData->Error)),
            settingsData->MinSymptoms, settingsData->MaxSymptoms);
        client.Symptoms.SetNum(symptomCount);
        clients.Add(client);
    }

    int demonsNum = FMath::RandRange(0, FMath::Min(settingsData->MaxDemons, clientsNum));
    int demonSymptomCount = FMath::Clamp(FMath::RoundToInt(expectedValue + FMath::RandRange(-settingsData->Error, settingsData->Error)),
        settingsData->MinSymptoms, settingsData->MaxSymptoms);
    demons = TArray<FClient>();
    for (int i = 0; i < demonsNum; ++i) {
        FClient demon;
        demon.IsDemon = true;
        demon.Symptoms.SetNum(demonSymptomCount);
        demons.Add(demon);
    }
}

TArray<FClient> ClientsGenerator::GenerateClients(const UGameSettings& GameSettings, const FClientsGeneratorData& ClientsGeneratorData)
{
    InitializeVariables(GameSettings, ClientsGeneratorData);
    UpdateSymptomPool();
    InitializeClients();

    return clients;
}