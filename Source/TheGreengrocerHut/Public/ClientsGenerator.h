#pragma once

#include "CoreMinimal.h"
#include "ClientStruct.h"
#include "GameSettings.h"
#include "GameProjectSettingsNew.h"
#include "SaveGameData.h"

class THEGREENGROCERHUT_API ClientsGenerator
{
public:
    ClientsGenerator(const UObject* WorldContextObject,
                    const UGameProjectSettings* ProjectSettings,
                    const TObjectPtr<UGameSettings> GameSettings,
                    const FDaySnapshot& CurrentSnapshot,
                    const FGameMetrics& GameMetrics);

    void Process(FDaySnapshot& OutSnapshot, FGameMetrics& OutMetrics);

private:
    void GenerateDemonSymptoms();

    void UpdateSymptomPool();
    void InitializeClients();
    void FillSymptoms();
    void GenerateSymptomsForClient(FClient& client);
    bool TryHandleTutorialDay();
    FName SelectSymptomFromPool(const TSet<EBodyPart>& occupiedParts, bool isDemon);

    FGameMetrics gameMetrics;
    FDaySnapshot daySnapshot;

    int demonsNum;
    int demonSymptomCount;

    float infectionRateNormalized;
    float progressionMultiplier;

    const UGameSettings* gameSettings;
    const UGameProjectSettings* projectSettings;
    const UObject* worldContextObject;
};
