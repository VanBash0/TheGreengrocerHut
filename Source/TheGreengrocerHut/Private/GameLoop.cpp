#include "GameLoop.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void UGameLoop::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    const UGameProjectSettings* ProjectSettings = GetDefault<UGameProjectSettings>();
    if (ProjectSettings)
    {
        GameSettings = ProjectSettings->GameSettingsAsset.LoadSynchronous();
    }

    LoadSave();

    ClientsGenerator clientsGenerator(this, ProjectSettings, GameSettings, _currentDaySnapshot, _metrics);
    clientsGenerator.Process(_currentDaySnapshot, _metrics);

    _currentDaySnapshot.DaySymptoms = ProjectSettings->SymptomTable->GetRowNames();

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimerForNextTick(this, &UGameLoop::TriggerDayStart);
    }
}

void UGameLoop::TriggerDayStart()
{
    SetNewState(EGameState::DayStart);
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

    EGameState oldState = CurrentState;
    CurrentState = NewState;

    OnGameStateChanged.Broadcast(oldState, NewState);
    TriggerStateEvent(CurrentState);
}

void UGameLoop::IncrementCurrentClient()
{
    if (CurrentState == EGameState::CompleteQuest)
    {
        if (GetWorld()->GetTimerManager().IsTimerActive(ClientSpawnTimerHandle)) { return; }

        if (currentClientIndex_ < _currentDaySnapshot.DayClients.Num() - 1)
        {
            SetNewState(EGameState::WaitForClient);

            float SpawnDelay = FMath::RandRange(GameSettings->WaitClientTimeRange.X, GameSettings->WaitClientTimeRange.Y);

            GetWorld()->GetTimerManager().SetTimer(
                ClientSpawnTimerHandle,
                [this]()
                {
                    currentClientIndex_++;
                    SetNewState(EGameState::SpawnClient);
                },
                SpawnDelay,
                false
            );
        }
        else
        {
            SetNewState(EGameState::DayEnd);
        }
    }
}

void UGameLoop::UpdateInfectionRate(float DeltaInfectionRate, bool IsGood)
{
    _currentDaySnapshot.VillageInfectionRate += DeltaInfectionRate;
    if (IsGood) {
        _metrics.HealingFactor += GameSettings->DeltaHealingFactor;
    }
    else {
        _metrics.KillingFactor += GameSettings->DeltaKillingFactor;
    }
}

void UGameLoop::LoadSave()
{
    USaveGame* loadedGame = UGameplayStatics::LoadGameFromSlot(TEXT("Save1"), 0);
    if (!loadedGame)
    {
        UE_LOG(LogTemp, Warning, TEXT("No save data found."));

        _currentDaySnapshot.VillageInfectionRate = 0;
        _metrics.DayNumber = 1;
        _metrics.HealingFactor = GameSettings->StartHealingFactor;
        _metrics.KillingFactor = GameSettings->StartKillingFactor;
        _metrics.HasDemonPrevious = false;
        _metrics.MaxClientSymptomCount = 0;

        _savedData = NewObject<USaveGameData>(this);

        return;
    }

    USaveGameData* save = Cast<USaveGameData>(loadedGame);
    if (!save)
    {
        UE_LOG(LogTemp, Error, TEXT("Loaded data is not of type USaveGameData"));
        return;
    }

    _savedData = save;

    if (save->DaySnapshots.Num() != 0)
    {
        _currentDaySnapshot.VillageInfectionRate = save->DaySnapshots.Last().VillageInfectionRate;
        _metrics = save->LastDayMetrics;
        _metrics.DayNumber++;
    }
    else
    {
        _currentDaySnapshot.VillageInfectionRate = 0;
        _metrics.DayNumber = 1;
        _metrics.HealingFactor = GameSettings->StartHealingFactor;
        _metrics.KillingFactor = GameSettings->StartKillingFactor;
        _metrics.HasDemonPrevious = false;
        _metrics.MaxClientSymptomCount = 0;
    }
}