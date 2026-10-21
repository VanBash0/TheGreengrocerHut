#include "GameLoop.h"
#include "ClientsGenerator.h"
#include "IngredientFunctionLibary.h"
#include "Engine/Engine.h"
#include "Algo/RandomShuffle.h"
#include "HAL/IConsoleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace PlayerQualitySimulation
{
    struct FProfile
    {
        const TCHAR* Name;
        float DemonRecognition;
        float RecipeAccuracy;
        float OrderAccuracy;
    };

    struct FDayStats
    {
        int64 Clients = 0;
        int64 Humans = 0;
        int64 Demons = 0;
        int64 Good = 0;
        int64 GoodHumans = 0;
        int64 GoodDemons = 0;
        int64 HumanSymptoms = 0;
        int64 DemonSymptoms = 0;
        int64 PoolSize = 0;
        double ValidFractionSum = 0.0;
        double DeltaSum = 0.0;
        double StartRateSum = 0.0;
        double EndingRateSum = 0.0;
    };

    int32 Priority(UCacheSubsystem* Cache, const FName& Name)
    {
        const FIngredient* Ingredient = Cache ? Cache->GetIngredientByRowName(Name) : nullptr;
        return Ingredient ? Ingredient->TermsOfUse.AddPriority : 0;
    }

    void BuildRecipe(const UObject* World, UCacheSubsystem* Cache, const UGameSettings* Settings, const FClient& Client, TArray<FName>& OutRecipe)
    {
        OutRecipe.Reset();
        TArray<FName> SymptomIngredients;
        UIngredientFunctionLibary::GetIngredientsBySymptoms(World, Client.Symptoms, SymptomIngredients);

        FName Base;
        if (Client.IsDemon)
        {
            Base = Settings->PoisonBase.RowName;
        }
        else
        {
            UIngredientFunctionLibary::GetBasePotionBySumptomCount(World, Settings, Client.Symptoms.Num(), Base);
        }

        if (!Base.IsNone()) OutRecipe.Add(Base);
        OutRecipe.Append(SymptomIngredients);
        OutRecipe.Sort([Cache](const FName& A, const FName& B) { return Priority(Cache, A) < Priority(Cache, B); });
    }

    float EvaluatePotion(const UObject* World, UCacheSubsystem* Cache, const UGameSettings* Settings, const FClient& ActualClient,
        const TArray<FName>& Given, int32 DayNumber, float HealingFactor, float KillingFactor, bool& bGood, float& OutDelta)
    {
        TArray<FName> Required;
        BuildRecipe(World, Cache, Settings, ActualClient, Required);
        const FName RequiredBase = Required.Num() > 0 ? Required[0] : NAME_None;

        TMap<FName, int32> Remaining;
        for (int32 Index = 1; Index < Required.Num(); ++Index) Remaining.FindOrAdd(Required[Index])++;

        int32 MatchingIngredients = 0;
        int32 CurrentPriority = TNumericLimits<int32>::Min();
        for (int32 Index = 0; Index < Given.Num(); ++Index)
        {
            const FName GivenName = Given[Index];
            bool bValid = false;
            if (Index == 0)
            {
                bValid = GivenName == RequiredBase && !GivenName.IsNone();
                if (bValid) CurrentPriority = TNumericLimits<int32>::Min();
            }
            else
            {
                int32* RemainingCount = Remaining.Find(GivenName);
                bValid = RemainingCount && *RemainingCount > 0 && !GivenName.IsNone() && Priority(Cache, GivenName) >= CurrentPriority;
                if (bValid)
                {
                    --(*RemainingCount);
                    CurrentPriority = FMath::Max(CurrentPriority, Priority(Cache, GivenName));
                }
            }
            if (bValid) ++MatchingIngredients;
        }

        const int32 TotalNeeded = Required.Num();
        const float ValidFraction = FMath::Max(Given.Num(), TotalNeeded) > 0
            ? static_cast<float>(MatchingIngredients) / static_cast<float>(FMath::Max(Given.Num(), TotalNeeded)) : 0.f;
        const bool bPoison = Given.Contains(Settings->PoisonBase.RowName);

        if (ActualClient.IsDemon)
        {
            bGood = bPoison || ValidFraction >= Settings->DemonStealThreshold;
        }
        else
        {
            bGood = !bPoison && ValidFraction >= Settings->HumanHealThreshold;
        }

        EPotionResult Result;
        if (ActualClient.IsDemon)
        {
            Result = bPoison ? EPotionResult::Demon_Poisoned : (ValidFraction >= Settings->DemonStealThreshold ? EPotionResult::Demon_GivenGoodPotion : EPotionResult::Demon_NotPoisoned);
        }
        else
        {
            Result = bPoison ? EPotionResult::Human_Poisoned : (ValidFraction >= Settings->HumanHealThreshold ? EPotionResult::Human_Healed : EPotionResult::Human_NotHealed);
        }

        const float DayMultiplier = 1.f + 0.02f * (DayNumber - 1);
        const float BaseDelta = Settings->PotionResultDeltaMap.FindRef(Result);
        OutDelta = BaseDelta;
        switch (Result)
        {
        case EPotionResult::Human_Healed: OutDelta *= DayMultiplier * HealingFactor * ValidFraction; break;
        case EPotionResult::Human_NotHealed: OutDelta *= DayMultiplier * KillingFactor * (1.f - ValidFraction); break;
        case EPotionResult::Human_Poisoned:
        case EPotionResult::Demon_Poisoned: OutDelta *= DayMultiplier * HealingFactor; break;
        case EPotionResult::Demon_NotPoisoned:
        case EPotionResult::Demon_GivenGoodPotion: OutDelta *= DayMultiplier * KillingFactor; break;
        }
        return ValidFraction;
    }

    void MakePlayerPotion(const UObject* World, UCacheSubsystem* Cache, const UGameSettings* Settings, const FClient& Client,
        const FProfile& Profile, TArray<FName>& OutGiven)
    {
        FClient PerceivedClient = Client;
        if (FMath::FRand() > Profile.DemonRecognition) PerceivedClient.IsDemon = !PerceivedClient.IsDemon;

        TArray<FName> Intended;
        BuildRecipe(World, Cache, Settings, PerceivedClient, Intended);
        OutGiven.Reset();

        TArray<FName> AllIngredients;
        if (Cache) Cache->GetIngredientCache().GenerateKeyArray(AllIngredients);
        for (const FName& Ingredient : Intended)
        {
            if (FMath::FRand() <= Profile.RecipeAccuracy)
            {
                OutGiven.Add(Ingredient);
            }
            else if (FMath::FRand() > 0.5f && AllIngredients.Num() > 0)
            {
                OutGiven.Add(AllIngredients[FMath::RandRange(0, AllIngredients.Num() - 1)]);
            }
        }

        if (FMath::FRand() > Profile.OrderAccuracy) Algo::RandomShuffle(OutGiven);
    }

    void Run(UWorld* World, int32 Days, int32 Runs, int32 Seed)
    {
        UGameLoop* GameLoop = World ? World->GetSubsystem<UGameLoop>() : nullptr;
        if (!GameLoop || !GameLoop->GameSettings || !World->GetGameInstance())
        {
            UE_LOG(LogTemp, Error, TEXT("[PlayerQualityTest] GameLoop, GameSettings or GameInstance is unavailable."));
            return;
        }

        const UGameProjectSettings* ProjectSettings = GetDefault<UGameProjectSettings>();
        UCacheSubsystem* Cache = World->GetGameInstance()->GetSubsystem<UCacheSubsystem>();
        if (!Cache)
        {
            UE_LOG(LogTemp, Error, TEXT("[PlayerQualityTest] Ingredient cache is unavailable."));
            return;
        }

        const TArray<FProfile> Profiles = {
            { TEXT("Dumb"), 0.55f, 0.50f, 0.55f },
            { TEXT("Average"), 0.80f, 0.78f, 0.82f },
            { TEXT("Smart"), 0.95f, 0.95f, 0.97f }
        };
        TArray<FDayStats> Stats;
        Stats.SetNum(Profiles.Num() * Days);

        for (int32 ProfileIndex = 0; ProfileIndex < Profiles.Num(); ++ProfileIndex)
        {
            for (int32 RunIndex = 0; RunIndex < Runs; ++RunIndex)
            {
                FMath::RandInit(static_cast<uint32>(Seed + ProfileIndex * 100000 + RunIndex));
                FGameMetrics Metrics;
                Metrics.DayNumber = 1;
                Metrics.HealingFactor = GameLoop->GameSettings->StartHealingFactor;
                Metrics.KillingFactor = GameLoop->GameSettings->StartKillingFactor;
                FDaySnapshot Input;
                Input.VillageInfectionRate = 0.f;

                for (int32 DayIndex = 0; DayIndex < Days; ++DayIndex)
                {
                    Metrics.DayNumber = DayIndex + 1;
                    FDaySnapshot Output;
                    FGameMetrics OutputMetrics;
                    ClientsGenerator Generator(GameLoop, ProjectSettings, GameLoop->GameSettings, Input, Metrics);
                    Generator.Process(Output, OutputMetrics);
                    Metrics = OutputMetrics;

                    FDayStats& DayStats = Stats[ProfileIndex * Days + DayIndex];
                    DayStats.StartRateSum += Input.VillageInfectionRate;
                    DayStats.PoolSize += OutputMetrics.SymptomMetrics.Num();
                    for (const FClient& Client : Output.DayClients)
                    {
                        TArray<FName> Given;
                        MakePlayerPotion(GameLoop, Cache, GameLoop->GameSettings, Client, Profiles[ProfileIndex], Given);
                        bool bGood = false;
                        float Delta = 0.f;
                        const float ValidFraction = EvaluatePotion(GameLoop, Cache, GameLoop->GameSettings, Client, Given, Metrics.DayNumber, Metrics.HealingFactor, Metrics.KillingFactor, bGood, Delta);
                        ++DayStats.Clients;
                        if (Client.IsDemon)
                        {
                            ++DayStats.Demons;
                            DayStats.DemonSymptoms += Client.Symptoms.Num();
                        }
                        else
                        {
                            ++DayStats.Humans;
                            DayStats.HumanSymptoms += Client.Symptoms.Num();
                        }
                        if (bGood)
                        {
                            ++DayStats.Good;
                            Client.IsDemon ? ++DayStats.GoodDemons : ++DayStats.GoodHumans;
                            Metrics.HealingFactor += GameLoop->GameSettings->DeltaHealingFactor;
                        }
                        else
                        {
                            Metrics.KillingFactor += GameLoop->GameSettings->DeltaKillingFactor;
                        }
                        DayStats.ValidFractionSum += ValidFraction;
                        DayStats.DeltaSum += Delta;
                        Input.VillageInfectionRate = FMath::Clamp(Input.VillageInfectionRate + Delta, -100.f, 100.f);
                    }
                    DayStats.EndingRateSum += Input.VillageInfectionRate;
                    Input.DayClients.Empty();
                    Input.DaySymptoms.Empty();
                    Input.DemonSymptoms.Empty();
                }
            }
        }

        FString GenerationCsv = TEXT("profile,day,rate,avg_clients,avg_humans,avg_demons,avg_human_symptoms,avg_demon_symptoms,avg_pool_size\r\n");
        FString QualityCsv = TEXT("profile,day,runs,good_rate,human_good_rate,demon_good_rate,avg_valid_fraction,avg_delta,ending_infection_rate\r\n");
        for (int32 ProfileIndex = 0; ProfileIndex < Profiles.Num(); ++ProfileIndex)
        {
            for (int32 DayIndex = 0; DayIndex < Days; ++DayIndex)
            {
                const FDayStats& S = Stats[ProfileIndex * Days + DayIndex];
                const float Count = static_cast<float>(FMath::Max<int64>(1, S.Clients));
                const float Humans = static_cast<float>(FMath::Max<int64>(1, S.Humans));
                const float Demons = static_cast<float>(FMath::Max<int64>(1, S.Demons));
                const float RunCount = static_cast<float>(FMath::Max(1, Runs));
                const float AvgHumanSymptoms = S.HumanSymptoms / Humans;
                const float AvgDemonSymptoms = S.DemonSymptoms / Demons;
                GenerationCsv += FString::Printf(TEXT("%s,%d,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\r\n"), Profiles[ProfileIndex].Name, DayIndex + 1,
                    S.StartRateSum / RunCount, S.Clients / RunCount, S.Humans / RunCount, S.Demons / RunCount, AvgHumanSymptoms, AvgDemonSymptoms, S.PoolSize / RunCount);
                QualityCsv += FString::Printf(TEXT("%s,%d,%d,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\r\n"), Profiles[ProfileIndex].Name, DayIndex + 1, Runs,
                    S.Good / Count, S.GoodHumans / Humans, S.GoodDemons / Demons, S.ValidFractionSum / Count, S.DeltaSum / Count, S.EndingRateSum / RunCount);
            }
        }

        const FString Directory = FPaths::ProjectSavedDir() / TEXT("PlayerQualityTests");
        IFileManager::Get().MakeDirectory(*Directory, true);
        const FString BaseName = FString::Printf(TEXT("PlayerQuality_%s"), *FDateTime::Now().ToString(TEXT("yyyyMMdd_HHmmss")));
        const FString CsvPath = Directory / (BaseName + TEXT(".csv"));
        const FString QualityCsvPath = Directory / (BaseName + TEXT("_quality.csv"));
        FFileHelper::SaveStringToFile(GenerationCsv, *CsvPath);
        FFileHelper::SaveStringToFile(QualityCsv, *QualityCsvPath);
        UE_LOG(LogTemp, Display, TEXT("[PlayerQualityTest] Finished. Generation-style CSV: %s"), *CsvPath);
        UE_LOG(LogTemp, Display, TEXT("[PlayerQualityTest] Quality CSV: %s"), *QualityCsvPath);
    }
}

static FAutoConsoleCommand GPlayerQualitySimulationCommand(
    TEXT("TheGreengrocer.TestPlayerQuality"),
    TEXT("Run player quality simulation. Args: Days Runs Seed"),
    FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
    {
        const int32 Days = Args.Num() > 0 ? FMath::Max(1, FCString::Atoi(*Args[0])) : 30;
        const int32 Runs = Args.Num() > 1 ? FMath::Max(1, FCString::Atoi(*Args[1])) : 100;
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
        PlayerQualitySimulation::Run(World, Days, Runs, Seed);
    }));
