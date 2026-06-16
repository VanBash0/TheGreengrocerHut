#pragma once

#include "CoreMinimal.h"
#include "GameLoop.h"
#include "ClientStruct.h"
#include "ClientsGenerator.generated.h"

USTRUCT(BlueprintType)
struct FClientsGeneratorData
{
    GENERATED_BODY()

    int DayNumber;
    float InfectionRate;
    TArray<FName> UnlockedSymptoms;
};

class THEGREENGROCERHUT_API ClientsGenerator
{
public:
    TArray<FClient> GenerateClients(const UGameSettings& GameSettings, const FClientsGeneratorData& ClientsGeneratorData);
private:
    void InitializeVariables(const UGameSettings& GameSettings, const FClientsGeneratorData& ClientsGeneratorData);
    void UpdateSymptomPool();

    float InfectionRateNormalized;
    float ProgressionMultiplier;
    TArray<FName> UnlockedSymptoms;
    FClientsGeneratorData GeneratorData;
    const UGameSettings* SettingsData;
};
