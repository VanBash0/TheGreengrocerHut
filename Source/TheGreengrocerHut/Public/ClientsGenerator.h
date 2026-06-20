#pragma once

#include "CoreMinimal.h"
#include "ClientStruct.h"
#include "GameSettings.h"
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
    TArray<FClient> GenerateClients(const UGameProjectSettings* ProjectSettings, const TObjectPtr<UGameSettings> GameSettings, FClientsGeneratorData& ClientsGeneratorData);
    TArray<FName> GetDemonSymptoms() { return demonSymptomsOfDay; }
private:
    void InitializeVariables(const UGameProjectSettings* ProjectSettings, const TObjectPtr<UGameSettings> GameSettings, FClientsGeneratorData& ClientsGeneratorData);
    void UpdateSymptomPool();
    void InitializeClients();
    void FillSymptoms();
    void GenerateSymptomsForClient(FClient& client);
    bool TryHandleTutorialDay();
    void GenerateDemonSymptoms(int symptomCount);
    FName SelectSymptomFromPool(const TSet<EBodyPart>& occupiedParts, bool isDemon);

    TArray<FClient> clients;
    TArray<FName> demonSymptomsOfDay;
    int demonsNum;
    int demonSymptomCount;

    float infectionRateNormalized;
    float progressionMultiplier;
    FClientsGeneratorData generatorData;
    const UGameSettings* gameSettings;
    const UGameProjectSettings* projectSettings;
    TMap<FName, FSymptomWithWeightsData> unlockedSymptoms;
};
