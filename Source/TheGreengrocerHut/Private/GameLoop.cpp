#include "GameLoop.h"
#include "Engine/World.h"

void UGameLoop::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    const UGameProjectSettings* ProjectSettings = GetDefault<UGameProjectSettings>();
    if (ProjectSettings)
    {
        GameSettings = ProjectSettings->GameSettingsAsset.LoadSynchronous();
    }
}

void UGameLoop::Deinitialize()
{
    Super::Deinitialize();
}

bool UGameLoop::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer))
        return false;

    UWorld* World = Cast<UWorld>(Outer);
    if (!World)
        return false;

    FString WorldName = World->GetName();

   /* if (WorldName.Contains(TEXT("GAME_LEVEL")))
        return true;*/

    return true;

    return false;
}

const TMap<EGameState, UGameLoop::StateDelegatePtr> UGameLoop::StateEventMap = InitStateEventMap();

TMap<EGameState, UGameLoop::StateDelegatePtr> UGameLoop::InitStateEventMap()
{
    TMap<EGameState, StateDelegatePtr> Map;
    Map.Add(EGameState::DayStart, &UGameLoop::OnDayStart);
    Map.Add(EGameState::WaitForClient, &UGameLoop::OnWaitClient);
    Map.Add(EGameState::SpawnClient, &UGameLoop::OnSpawnClient);
    Map.Add(EGameState::CompleteQuest, &UGameLoop::OnCompleteQuest);
    Map.Add(EGameState::DayEnd, &UGameLoop::OnDayEnd);
    Map.Add(EGameState::GameEnd, &UGameLoop::OnGameEnd);

    return Map;
}

void UGameLoop::TriggerStateEvent(EGameState State)
{
    const StateDelegatePtr* FoundPtr = StateEventMap.Find(State);
    if (FoundPtr)
    {
        StateDelegatePtr DelegatePtr = *FoundPtr;
        (this->*DelegatePtr).Broadcast();
    }
}

void UGameLoop::SetNewState(EGameState NewState)
{
    if (NewState == CurrentState) { return; }

    CurrentState = NewState;
    OnGameStateChanged.Broadcast();
    TriggerStateEvent(CurrentState);
}