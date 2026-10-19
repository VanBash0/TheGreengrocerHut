#pragma once

#include "CoreMinimal.h"
#include "ClientStruct.h"
#include "GameSettings.h"
#include "GameProjectSettings.h"
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
    bool TryHandleTutorialDay();

private:
    void UpdateSymptomPool();
    void InitializeClients();
    void FillSymptoms();

    void GenerateDemonSymptoms();
    void GenerateSymptomsForClient(FClient& client);
    void EnsureSymptomsDontMatchDemon(FClient& client);

    FName SelectSymptomFromPool(const TSet<EBodyPart>& occupiedParts, bool isDemon);

private:
    void AssignGuests();
    void PickGuestsForClient(FClient& client, const TArray<const UGuestData*>& pool, TSet<FName>& usedToday);
    const UGuestData* SelectGuest(const TArray<const UGuestData*>& pool, bool isDemon, const TSet<FName>& usedToday) const;

    TArray<const UGuestData*> LoadGuestPool() const;
    bool IsGuestAvailable(const UGuestData* guest, bool isDemon) const;
    float GetGuestWeight(const UGuestData* guest, bool isDemon) const;

    void ValidateTutorialGuests() const;

private:
    const UObject* worldContextObject = nullptr;
    const UGameProjectSettings* projectSettings = nullptr;
    const UGameSettings* gameSettings = nullptr;

private:
    FGameMetrics gameMetrics;
    FDaySnapshot daySnapshot;

private:
    float infectionRateNormalized = 0.f;
    float progressionMultiplier = 0.f;

    int demonsNum = 0;
    int demonSymptomCount = 0;
};
