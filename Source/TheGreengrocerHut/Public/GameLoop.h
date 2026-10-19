#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameSettings.h"
#include "GameProjectSettings.h"
#include "ClientsGenerator.h"
#include "ClientStruct.h"
#include "SaveGameData.h"
#include "GameLoop.generated.h"

UENUM(BlueprintType)
enum class EGameState : uint8
{
    None UMETA(DisplayName = "None"),
    DayStart UMETA(DisplayName = "Day Start"),
    WaitForClient UMETA(DisplayName = "Wait For Client"),
    SpawnClient UMETA(DisplayName = "Spawn Client"),
    CompleteQuest UMETA(DisplayName = "Complete Quest"),
    DayEnd UMETA(DisplayName = "Day End"),
    GameEnd UMETA(DisplayName = "Game End")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGameStateChangedWithArgs, EGameState, OldState, EGameState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGameStateChanged);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnReputationChanged, EGuestFraction, Fraction, int32, Delta, int32, NewValue);

UCLASS(BlueprintType, Blueprintable)
class THEGREENGROCERHUT_API UGameLoop : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    UFUNCTION(BlueprintCallable, Category = "GameLoop")
    void SetNewState(EGameState NewState);

    UPROPERTY(BlueprintReadOnly)
    EGameState CurrentState = EGameState::None;

    UPROPERTY(BlueprintAssignable) FGameStateChangedWithArgs OnGameStateChanged;

    UPROPERTY(BlueprintAssignable) FGameStateChanged OnDayStart;
    UPROPERTY(BlueprintAssignable) FGameStateChanged OnWaitClient;
    UPROPERTY(BlueprintAssignable) FGameStateChanged OnSpawnClient;
    UPROPERTY(BlueprintAssignable) FGameStateChanged OnCompleteQuest;
    UPROPERTY(BlueprintAssignable) FGameStateChanged OnDayEnd;
    UPROPERTY(BlueprintAssignable) FGameStateChanged OnGameEnd;

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Client")
    void GetCurrentClient(FClient& client) const { client = _currentDaySnapshot.DayClients[currentClientIndex_]; }

    UFUNCTION(BlueprintCallable, Category = "Client")
    void IncrementCurrentClient();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Client")
    bool IsFirstClient() const { return currentClientIndex_ == 0; }

public:
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void GetDaySnapshot(FDaySnapshot& OutSnapshot) const { OutSnapshot = _currentDaySnapshot; }

    UFUNCTION(BlueprintCallable, BlueprintPure)
    void GetGameMetrics(FGameMetrics& OutMetrics) const { OutMetrics = _metrics; }

    UFUNCTION(BlueprintCallable, BlueprintPure)
    USaveGameData* GetSavedData() const { return _savedData.Get(); }

    UFUNCTION(BlueprintCallable, Category = "Save")
    const FGameMetrics GetPreviousDayMetrics() const { return _savedData->LastDayMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Save")
    const TArray<FName> GetPreviousDaySymptoms() const { return _savedData->DaySnapshots.Num() > 0 ? _savedData->DaySnapshots.Last().DaySymptoms : TArray<FName>(); }

public:
    UFUNCTION(BlueprintPure, Category = "Dialog")
    bool TryGetGuestMetrics(FName Key, FGuestMetrics& Out) const;

    UFUNCTION(BlueprintPure, Category = "Dialog")
    FGuestMetrics GetGuestMetrics(const UGuestData* Data) const;

    UFUNCTION(BlueprintCallable, Category = "Dialog")
    void SetGuestMetrics(const UGuestData* Data, const FGuestMetrics& Metrics);

public:
    UFUNCTION(BlueprintPure, Category = "Reputation")
    int32 GetReputation(EGuestFraction Fraction) const;

    UPROPERTY(BlueprintAssignable, Category = "Reputation")
    FOnReputationChanged OnReputationChanged;

public:
    UFUNCTION(BlueprintCallable, Category = "Update Stats")
    void UpdateInfectionRate(float DeltaInfectionRate, bool IsGood);

    UFUNCTION(BlueprintCallable, Category = "Update Stats")
    FGuestOutcomeResult UpdateGuestOutcome(const UGuestData* Guest, EGuestOutcome Outcome, const TArray<FGuestReputationModifier>& Modifiers);

    // Counts one more visit of the guest (call once per arrival). Not exposed to BP on purpose.
    void RegisterGuestVisit(const UGuestData* Guest);

public:
    UFUNCTION(BlueprintCallable, Category = "Save")
    void SaveGame();

    UFUNCTION(BlueprintCallable, Category = "Save")
    void ResetGameData();

public:
    UPROPERTY(BlueprintReadOnly, Category = "Settings")
    TObjectPtr<UGameSettings> GameSettings;

private:
    using StateDelegatePtr = FGameStateChanged UGameLoop::*;
    static const TMap<EGameState, StateDelegatePtr> StateEventMap;
    static TMap<EGameState, StateDelegatePtr> InitStateEventMap();

    void TriggerStateEvent(EGameState State);
    void TriggerDayStart();

    void LoadSave();
    FGameMetrics GetDefaultGameMetrics() const;

private:
    FDaySnapshot _currentDaySnapshot;
    FGameMetrics _metrics;

    int currentClientIndex_ = 0;

private:
    UPROPERTY()
    TObjectPtr<USaveGameData> _savedData;

private:
    FTimerHandle ClientSpawnTimerHandle;

private:
    inline static const FString SaveSlotName = TEXT("GAME_LOOP_SAVE_SLOT");
};