#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
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
    void SetNewState(const EGameState& NewState);

    UPROPERTY(BlueprintReadOnly)
    EGameState CurrentState = EGameState::None;

    UPROPERTY(BlueprintAssignable) FGameStateChanged OnGameStateChanged;
    UPROPERTY(BlueprintAssignable) FGameStateChanged OnDayStart;
    UPROPERTY(BlueprintAssignable) FGameStateChanged OnWaitClient;
    UPROPERTY(BlueprintAssignable) FGameStateChanged OnSpawnClient;
    UPROPERTY(BlueprintAssignable) FGameStateChanged OnCompleteQuest;
    UPROPERTY(BlueprintAssignable) FGameStateChanged OnDayEnd;
    UPROPERTY(BlueprintAssignable) FGameStateChanged OnGameEnd;

private:
    using StateDelegatePtr = FGameStateChanged UGameLoop::*;
    static const TMap<EGameState, StateDelegatePtr> StateEventMap;
    static TMap<EGameState, StateDelegatePtr> InitStateEventMap();
    void TriggerStateEvent(EGameState State);
};