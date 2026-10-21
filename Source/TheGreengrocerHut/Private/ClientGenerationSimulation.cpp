#include "GameLoop.h"
#include "ClientsGenerator.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace ClientGenerationSimulation
{
    enum class ERateScenario : uint8 { Rise, Fall, RandomWalkByDay };

    const TCHAR* Name(ERateScenario Scenario)
    {
        switch (Scenario)
        {
        case ERateScenario::Rise: return TEXT("Rise_0_to_+100");
        case ERateScenario::Fall: return TEXT("Fall_0_to_-100");
        case ERateScenario::RandomWalkByDay: return TEXT("RandomWalk_plus_minus_day");
        default: return TEXT("Unknown");
        }
    }

    float Rate(ERateScenario Scenario, int32 DayIndex, int32 Days)
    {
        if (Days <= 1)
        {
            return 0.f;
        }

        const float Alpha = static_cast<float>(DayIndex) / static_cast<float>(Days - 1);
        switch (Scenario)
        {
        case ERateScenario::Rise: return FMath::Lerp(0.f, 100.f, Alpha);
        case ERateScenario::Fall: return FMath::Lerp(0.f, -100.f, Alpha);
        default: return 0.f;
        }
    }

    FString Join(const TArray<FName>& Names)
    {
        FString Result;
        for (int32 Index = 0; Index < Names.Num(); ++Index)
        {
            if (Index > 0) Result += TEXT(", ");
            Result += Names[Index].ToString();
        }
        return Result.IsEmpty() ? TEXT("none") : Result;
    }

    struct FDayAggregate
    {
        int64 Clients = 0;
        int64 Humans = 0;
        int64 Demons = 0;
        int64 HumanSymptoms = 0;
        int64 DemonSymptoms = 0;
        int64 PoolSize = 0;
        int64 Samples = 0;
        double RateSum = 0.0;
    };

    void Run(UWorld* World, int32 Days, int32 Runs, int32 Seed)
    {
        UGameLoop* GameLoop = World ? World->GetSubsystem<UGameLoop>() : nullptr;
        if (!GameLoop || !GameLoop->GameSettings)
        {
            UE_LOG(LogTemp, Error, TEXT("[ClientGenerationTest] GameLoop or GameSettings is unavailable."));
            return;
        }

        const UGameProjectSettings* ProjectSettings = GetDefault<UGameProjectSettings>();
        UDataTable* TutorialDaysTable = ProjectSettings ? ProjectSettings->TutorialDaysTable.LoadSynchronous() : nullptr;

        Days = FMath::Max(1, Days);
        Runs = FMath::Max(1, Runs);
        const TArray<ERateScenario> Scenarios = { ERateScenario::RandomWalkByDay, ERateScenario::Rise, ERateScenario::Fall };

        FString Detailed = FString::Printf(TEXT("TheGreengrocerHut client generation simulation\r\nDays=%d Runs=%d Seed=%d\r\n\r\n"), Days, Runs, Seed);
        FString Summary = TEXT("scenario,day,rate,avg_clients,avg_humans,avg_demons,avg_human_symptoms,avg_demon_symptoms,avg_pool_size\r\n");

        for (int32 ScenarioIndex = 0; ScenarioIndex < Scenarios.Num(); ++ScenarioIndex)
        {
            const ERateScenario Scenario = Scenarios[ScenarioIndex];
            TArray<FDayAggregate> Aggregates;
            Aggregates.SetNum(Days);
            TMap<FName, int32> SymptomOccurrences;
            Detailed += FString::Printf(TEXT("================ SCENARIO: %s ================\r\n"), Name(Scenario));

            const int32 ScenarioRuns = Scenario == ERateScenario::RandomWalkByDay ? 1 : Runs;
            for (int32 RunIndex = 0; RunIndex < ScenarioRuns; ++RunIndex)
            {
                FMath::RandInit(static_cast<uint32>(Seed + ScenarioIndex * 100000 + RunIndex));
                FGameMetrics Metrics;
                Metrics.DayNumber = 1;
                Metrics.HealingFactor = GameLoop->GameSettings->StartHealingFactor;
                Metrics.KillingFactor = GameLoop->GameSettings->StartKillingFactor;
                FDaySnapshot Input;
                TSet<FName> PreviousPool;
                float CurrentRate = 0.f;
                bool bRunAborted = false;
                Detailed += FString::Printf(TEXT("\r\n-- Run %d --\r\n"), RunIndex + 1);

                for (int32 DayIndex = 0; DayIndex < Days; ++DayIndex)
                {
                    if (Scenario == ERateScenario::RandomWalkByDay)
                    {
                        const float Delta = static_cast<float>(DayIndex + 1) * (FMath::RandBool() ? 1.f : -1.f);
                        CurrentRate += Delta;
                        if (FMath::Abs(CurrentRate) > 100.f)
                        {
                            Detailed += FString::Printf(TEXT("ABORTED on day %02d: infection rate %.2f exceeded [-100; 100] after delta %+.2f\r\n"),
                                DayIndex + 1, CurrentRate, Delta);
                            bRunAborted = true;
                            break;
                        }
                        Input.VillageInfectionRate = CurrentRate;
                    }
                    else
                    {
                        Input.VillageInfectionRate = FMath::Clamp(Rate(Scenario, DayIndex, Days), -100.f, 100.f);
                    }
                    Metrics.DayNumber = DayIndex + 1;
                    FDaySnapshot Output;
                    FGameMetrics OutputMetrics;
                    ClientsGenerator Generator(GameLoop, ProjectSettings, GameLoop->GameSettings, Input, Metrics);
                    Generator.Process(Output, OutputMetrics);
                    const bool bTutorialDay = TutorialDaysTable && TutorialDaysTable->FindRow<FTutorialDay>(FName(FString::FromInt(DayIndex + 1)), TEXT("ClientGenerationSimulation")) != nullptr;

                    int32 DemonCount = 0;
                    for (const FClient& Client : Output.DayClients)
                    {
                        DemonCount += Client.IsDemon ? 1 : 0;
                        if (Client.IsDemon)
                        {
                            Aggregates[DayIndex].DemonSymptoms += Client.Symptoms.Num();
                        }
                        else
                        {
                            Aggregates[DayIndex].HumanSymptoms += Client.Symptoms.Num();
                        }
                        for (const FName& Symptom : Client.Symptoms) SymptomOccurrences.FindOrAdd(Symptom)++;
                    }

                    TArray<FName> Pool;
                    OutputMetrics.SymptomMetrics.GenerateKeyArray(Pool);
                    Pool.Sort([](const FName& A, const FName& B) { return A.ToString() < B.ToString(); });
                    TArray<FName> NewSymptoms;
                    for (const FName& Symptom : Pool) if (!PreviousPool.Contains(Symptom)) NewSymptoms.Add(Symptom);
                    PreviousPool.Empty();
                    for (const FName& Symptom : Pool) PreviousPool.Add(Symptom);

                    FDayAggregate& Aggregate = Aggregates[DayIndex];
                    Aggregate.Clients += Output.DayClients.Num();
                    Aggregate.Humans += Output.DayClients.Num() - DemonCount;
                    Aggregate.Demons += DemonCount;
                    Aggregate.PoolSize += Pool.Num();
                    Aggregate.Samples++;
                    Aggregate.RateSum += Input.VillageInfectionRate;

                    Detailed += FString::Printf(TEXT("Day %02d | mode=%s | infection=%7.2f | clients=%d humans=%d demons=%d | pool=%d | new_symptoms=[%s]\r\n"),
                        DayIndex + 1, bTutorialDay ? TEXT("TUTORIAL") : TEXT("PROCEDURAL"), Input.VillageInfectionRate, Output.DayClients.Num(), Output.DayClients.Num() - DemonCount, DemonCount, Pool.Num(), *Join(NewSymptoms));
                    Detailed += FString::Printf(TEXT("  pool=[%s]\r\n"), *Join(Pool));
                    for (int32 ClientIndex = 0; ClientIndex < Output.DayClients.Num(); ++ClientIndex)
                    {
                        const FClient& Client = Output.DayClients[ClientIndex];
                        Detailed += FString::Printf(TEXT("  client=%02d | type=%s | symptoms=[%s]\r\n"), ClientIndex + 1,
                            Client.IsDemon ? TEXT("DEMON") : TEXT("HUMAN"), *Join(Client.Symptoms));
                    }

                    Metrics = OutputMetrics;
                    Input = FDaySnapshot();
                }

                if (bRunAborted)
                {
                    continue;
                }
            }

            Detailed += TEXT("\r\nScenario averages:\r\n");
            for (int32 DayIndex = 0; DayIndex < Days; ++DayIndex)
            {
                const FDayAggregate& Aggregate = Aggregates[DayIndex];
                const float Divisor = static_cast<float>(FMath::Max<int64>(1, Aggregate.Samples));
                const float AverageRate = static_cast<float>(Aggregate.RateSum / Divisor);
                const float AvgClients = Aggregate.Clients / Divisor;
                const float AvgHumans = Aggregate.Humans / Divisor;
                const float AvgDemons = Aggregate.Demons / Divisor;
                const float AvgHumanSymptoms = Aggregate.HumanSymptoms / static_cast<float>(FMath::Max<int64>(1, Aggregate.Humans));
                const float AvgDemonSymptoms = Aggregate.DemonSymptoms / static_cast<float>(FMath::Max<int64>(1, Aggregate.Demons));
                const float AvgPool = Aggregate.PoolSize / Divisor;
                Summary += FString::Printf(TEXT("%s,%d,%.2f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\r\n"), Name(Scenario), DayIndex + 1, AverageRate, AvgClients, AvgHumans, AvgDemons, AvgHumanSymptoms, AvgDemonSymptoms, AvgPool);
                Detailed += FString::Printf(TEXT("Day %02d | infection=%7.2f | samples=%lld | avg_clients=%.2f avg_humans=%.2f avg_demons=%.2f avg_human_symptoms=%.2f avg_demon_symptoms=%.2f avg_pool=%.2f\r\n"), DayIndex + 1, AverageRate, Aggregate.Samples, AvgClients, AvgHumans, AvgDemons, AvgHumanSymptoms, AvgDemonSymptoms, AvgPool);
            }
            Detailed += TEXT("\r\nSymptom occurrence frequency across all generated clients:\r\n");
            for (const auto& Pair : SymptomOccurrences) Detailed += FString::Printf(TEXT("  %s = %d\r\n"), *Pair.Key.ToString(), Pair.Value);
            Detailed += TEXT("\r\n");
        }

        const FString Directory = FPaths::ProjectSavedDir() / TEXT("ClientGenerationTests");
        IFileManager::Get().MakeDirectory(*Directory, true);
        const FString BaseName = FString::Printf(TEXT("ClientGeneration_%s"), *FDateTime::Now().ToString(TEXT("yyyyMMdd_HHmmss")));
        const FString DetailedLogFile = Directory / (BaseName + TEXT(".log"));
        const FString SummaryCsvFile = Directory / (BaseName + TEXT("_summary.csv"));
        FFileHelper::SaveStringToFile(Detailed, *DetailedLogFile);
        FFileHelper::SaveStringToFile(Summary, *SummaryCsvFile);
        UE_LOG(LogTemp, Display, TEXT("[ClientGenerationTest] Finished. Detailed log: %s"), *DetailedLogFile);
        UE_LOG(LogTemp, Display, TEXT("[ClientGenerationTest] Summary CSV: %s"), *SummaryCsvFile);
    }
}

static FAutoConsoleCommand GClientGenerationSimulationCommand(
    TEXT("TheGreengrocer.TestClientGeneration"),
    TEXT("Run client generation simulation. Args: Days Runs Seed"),
    FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
    {
        const int32 Days = Args.Num() > 0 ? FMath::Max(1, FCString::Atoi(*Args[0])) : 30;
        const int32 Runs = Args.Num() > 1 ? FMath::Max(1, FCString::Atoi(*Args[1])) : 10;
        const int32 Seed = Args.Num() > 2 ? FCString::Atoi(*Args[2]) : 1337;
        UWorld* World = nullptr;
        if (GEngine)
        {
            for (const FWorldContext& Context : GEngine->GetWorldContexts())
            {
                if (Context.World() && (Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game))
                {
                    World = Context.World();
                    break;
                }
            }
        }
        ClientGenerationSimulation::Run(World, Days, Runs, Seed);
    }));
