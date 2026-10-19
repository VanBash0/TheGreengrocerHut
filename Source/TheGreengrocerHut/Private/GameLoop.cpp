#include "GameLoop.h"
#include <IngredientFunctionLibary.h>
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DlgManager.h"
#include "Dialog/ReputationLibrary.h"

void UGameLoop::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UWorld* World = GetWorld();
    if (!World) { return; }

    const UGameProjectSettings* ProjectSettings = GetDefault<UGameProjectSettings>();
    if (ProjectSettings)
    {
        GameSettings = ProjectSettings->GameSettingsAsset.LoadSynchronous();
    }

    if (!GameSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("GameSettings asset failed to load! Check GameSettingsAsset in Project Settings and cook configuration."));
        return;
    }

    LoadSave();

    FString MapName = World->GetMapName();
    if (!MapName.Contains(TEXT("GAME_LEVEL"))) { return; }

    ClientsGenerator clientsGenerator(this, ProjectSettings, GameSettings, _currentDaySnapshot, _metrics);
    clientsGenerator.Process(_currentDaySnapshot, _metrics);

    World->GetTimerManager().SetTimerForNextTick(this, &UGameLoop::TriggerDayStart);
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
    if (!Super::ShouldCreateSubsystem(Outer)) { return false; }
    UWorld* World = Cast<UWorld>(Outer);
    if (!World) { return false; }
    EWorldType::Type WorldType = World->WorldType;
    return WorldType == EWorldType::PIE || WorldType == EWorldType::Game;
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

    UE_LOG(LogTemp, Warning, TEXT("SetNewState: %s -> %s"),
        *UEnum::GetValueAsString(CurrentState),
        *UEnum::GetValueAsString(NewState));

    UE_LOG(LogTemp, Warning, TEXT("%s"), *FFrame::GetScriptCallstack());

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

                    if (_currentDaySnapshot.DayClients.IsValidIndex(currentClientIndex_))
                    {
                        UReputationLibrary::LogClientArrival(currentClientIndex_, _currentDaySnapshot.DayClients[currentClientIndex_]);
                    }

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
    if (IsGood)
    {
        _metrics.HealingFactor += GameSettings->DeltaHealingFactor;
    }
    else {
        _metrics.KillingFactor += GameSettings->DeltaKillingFactor;
    }
}

FGuestOutcomeResult UGameLoop::UpdateGuestOutcome(const UGuestData* Guest, EGuestOutcome Outcome, const TArray<FGuestReputationModifier>& Modifiers)
{
    if (!Guest || !_savedData || !GameSettings) { return FGuestOutcomeResult(); }

    const bool bIsDemon = _currentDaySnapshot.DayClients.IsValidIndex(currentClientIndex_) && _currentDaySnapshot.DayClients[currentClientIndex_].IsDemon;

    FGuestMetrics Metrics = GetGuestMetrics(Guest);
    const FGuestOutcomeResult Result = UReputationLibrary::ApplyOutcome(GameSettings, Guest, bIsDemon, Outcome, Modifiers, Metrics, _savedData->Reputation);
    SetGuestMetrics(Guest, Metrics);

    for (const FReputationChange& Change : Result.ReputationChanges)
    {
        OnReputationChanged.Broadcast(Change.Fraction, Change.Delta, Change.NewValue);
    }

    return Result;
}

void UGameLoop::LoadSave()
{
    USaveGameData* save = Cast<USaveGameData>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
    if (!save)
    {
        UE_LOG(LogTemp, Warning, TEXT("No valid save data found, starting a new game."));
        save = NewObject<USaveGameData>(this);
    }

    _savedData = save;

    if (save->DaySnapshots.IsEmpty())
    {
        _currentDaySnapshot.VillageInfectionRate = 0;
        _metrics = GetDefaultGameMetrics();
    }
    else
    {
        _currentDaySnapshot.VillageInfectionRate = save->DaySnapshots.Last().VillageInfectionRate;
        _metrics = save->LastDayMetrics;
        _metrics.DayNumber++;
    }

    UDlgManager::SetDialogueHistory(save->DialogHistory);
}

void UGameLoop::SaveGame()
{
    if (!_savedData)
    {
        UE_LOG(LogTemp, Error, TEXT("[GameLoop] SaveGame: _savedData is null, nothing to save."));
        return;
    }

    _savedData->DaySnapshots.Add(_currentDaySnapshot);
    _savedData->LastDayMetrics = _metrics;

    _savedData->DialogHistory = UDlgManager::GetDialogueHistory();

    const bool bSaved = UGameplayStatics::SaveGameToSlot(_savedData, SaveSlotName, 0);
    if (!bSaved)
    {
        UE_LOG(LogTemp, Error, TEXT("[GameLoop] SaveGame: SaveGameToSlot failed for slot '%s'."), *SaveSlotName);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("[GameLoop] SaveGame: saved to slot '%s'."), *SaveSlotName);
    }
}

void UGameLoop::ResetGameData()
{
    UGameplayStatics::DeleteGameInSlot(SaveSlotName, 0);

    _savedData = NewObject<USaveGameData>(this);

    UDlgManager::ClearDialogueHistory();
}

FGameMetrics UGameLoop::GetDefaultGameMetrics() const
{
    FGameMetrics metrics = FGameMetrics();

    metrics.DayNumber = 1;
    metrics.HealingFactor = GameSettings->StartHealingFactor;
    metrics.KillingFactor = GameSettings->StartKillingFactor;
    metrics.HasDemonPrevious = false;
    metrics.MaxClientSymptomCount = 0;

    return metrics;
}

bool UGameLoop::TryGetGuestMetrics(FName Key, FGuestMetrics& Out) const
{
    Out = FGuestMetrics();

    if (!_savedData) { return false; }

    const FGuestMetrics* Found = _savedData->GuestMetrics.Find(Key);

    if (Found)
    {
        Out = *Found;

        return true;
    }
    else
    {
        return false;
    }
}

FGuestMetrics UGameLoop::GetGuestMetrics(const UGuestData* Data) const
{
    FGuestMetrics Result = {};
    if (!Data) { return Result; }

    if (TryGetGuestMetrics(Data->GetFName(), Result)) { return Result; }

    Result = Data->DefaultMetrics;

    return Result;
}

void UGameLoop::SetGuestMetrics(const UGuestData* Data, const FGuestMetrics& Metrics)
{
    if (!Data || !_savedData) { return; }

    _savedData->GuestMetrics.Add(Data->GetFName(), Metrics);
}

void UGameLoop::RegisterGuestVisit(const UGuestData* Guest)
{
    if (!Guest || !_savedData) { return; }

    FGuestMetrics Metrics = GetGuestMetrics(Guest);
    ++Metrics.VisitCount;
    SetGuestMetrics(Guest, Metrics);
}

int32 UGameLoop::GetReputation(EGuestFraction Fraction) const
{
    if (!_savedData) { return 0; }
    
    int32* Found = _savedData->Reputation.Find(Fraction);
    return Found ? *Found : 0;
}
