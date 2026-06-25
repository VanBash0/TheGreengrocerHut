#pragma once

#include "CoreMinimal.h"
#include "ClientStruct.h"
#include "GameSettings.h"
#include "SaveGameData.h"
#include "ClientsGenerator.generated.h"

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
    ClientsGenerator(const UObject* WorldContextObject, 
                    const UGameProjectSettings* ProjectSettings,
                    const TObjectPtr<UGameSettings> GameSettings,
                    FClientsGeneratorData& ClientsGeneratorData);

    void Process(FDaySnapshot& OutSnapshot);

private:
    void GenerateDemonSymptoms();

    void UpdateSymptomPool();
    void InitializeClients();
    void FillSymptoms();
    void GenerateSymptomsForClient(FClient& client);
    bool TryHandleTutorialDay();
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
    const UObject* worldContextObject;
    TMap<FName, FSymptomWithWeightsData> unlockedSymptoms;
};
