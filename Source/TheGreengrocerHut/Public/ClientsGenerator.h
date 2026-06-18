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

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EBodyPart BodyPart = EBodyPart::None;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float Weight = 1.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float DemonWeight = 1.0f;
};

USTRUCT(BlueprintType)
struct FClientsGeneratorData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int DayNumber;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float InfectionRate;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
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
