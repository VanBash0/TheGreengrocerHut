#include "ClientsGenerator.h"

void ClientsGenerator::InitializeVariables(const UGameSettings& GameSettings, const FClientsGeneratorData& ClientsGeneratorData)
{
    GeneratorData = ClientsGeneratorData;
    SettingsData = &GameSettings;
    InfectionRateNormalized = (GeneratorData.InfectionRate + 100.f) / 200.f;
    ProgressionMultiplier = 1 / (1 + FMath::Exp(-1 * SettingsData->Steepness * (GeneratorData.DayNumber - SettingsData->Midpoint)));
}

void ClientsGenerator::UpdateSymptomPool()
{
    int NewSymptomsNum = FMath::RandRange(1, SettingsData->MaxNewSymptoms);
}

TArray<FClient> ClientsGenerator::GenerateClients(const UGameSettings& GameSettings, const FClientsGeneratorData& ClientsGeneratorData)
{
    InitializeVariables(GameSettings, ClientsGeneratorData);
    UpdateSymptomPool();

    return TArray<FClient>();
}