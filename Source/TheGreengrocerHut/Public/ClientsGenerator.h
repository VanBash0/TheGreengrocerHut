#pragma once

#include "CoreMinimal.h"
#include "GameLoop.h"
#include "ClientStruct.h"
#include "SymptomStructures.h"
#include "ClientsGenerator.generated.h"

USTRUCT(BlueprintType)
struct FSymptomWithWeightsData
{
    GENERATED_BODY()

    EBodyPart BodyPart;
    float Weight;
    float DemonWeight;
};

USTRUCT(BlueprintType)
struct FClientsGeneratorData
{
    GENERATED_BODY()

    int DayNumber;
    float InfectionRate;
    TMap<FName, FSymptomWithWeightsData> UnlockedSymptoms;
};

class THEGREENGROCERHUT_API ClientsGenerator
{
public:
    TArray<FClient> GenerateClients(const UGameProjectSettings& ProjectSettings, const UGameSettings& GameSettings,
        const FClientsGeneratorData& ClientsGeneratorData);
private:
    void InitializeVariables(const UGameProjectSettings& ProjectSettings, const UGameSettings& GameSettings,
        const FClientsGeneratorData& ClientsGeneratorData);
    void UpdateSymptomPool();
    void InitializeClients();
    void FillSymptoms();

    bool TryHandleTutorialDay();

    TArray<FClient> clients;
    TArray<FClient> demons;

    float infectionRateNormalized;
    float progressionMultiplier;
    FClientsGeneratorData generatorData;
    const UGameSettings* gameSettings;
    const UGameProjectSettings* projectSettings;
    TMap<FName, FSymptomWithWeightsData> unlockedSymptoms;
};
