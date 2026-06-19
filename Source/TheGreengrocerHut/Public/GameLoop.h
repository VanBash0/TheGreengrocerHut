#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DeveloperSettings.h"
#include "GameSettings.h"
#include "ClientStruct.h"
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGameStateChanged);

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

    UPROPERTY(BlueprintAssignable) FGameStateChanged OnGameStateChanged;
    UPROPERTY(BlueprintAssignable) FGameStateChanged OnDayStart;
    UPROPERTY(BlueprintAssignable) FGameStateChanged OnWaitClient;
    UPROPERTY(BlueprintAssignable) FGameStateChanged OnSpawnClient;
    UPROPERTY(BlueprintAssignable) FGameStateChanged OnCompleteQuest;
    UPROPERTY(BlueprintAssignable) FGameStateChanged OnDayEnd;
    UPROPERTY(BlueprintAssignable) FGameStateChanged OnGameEnd;

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Client")
    FClient GetCurrentClient() const { return clients_[currentClientIndex_]; }

    UFUNCTION(BlueprintCallable, Category = "Client")
    void IncrementCurrentClient();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Client")
    bool IsFirstClient() const { return currentClientIndex_ == 0; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Client")
    TArray<FName> GetDayDemonSymptoms() const { return dayDemonSymptoms_; }

public:
    UPROPERTY(BlueprintReadOnly, Category = "Settings")
    TObjectPtr<UGameSettings> GameSettings;

public:

private:
    using StateDelegatePtr = FGameStateChanged UGameLoop::*;
    static const TMap<EGameState, StateDelegatePtr> StateEventMap;
    static TMap<EGameState, StateDelegatePtr> InitStateEventMap();
    void TriggerStateEvent(EGameState State);

private:
    TArray<FClient> clients_;
    int currentClientIndex_ = 0;

    TArray<FName> dayDemonSymptoms_;

private:
    FTimerHandle ClientSpawnTimerHandle;
};