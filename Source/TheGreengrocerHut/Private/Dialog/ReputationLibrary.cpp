#include "Dialog/ReputationLibrary.h"

#include "GameSettings.h"

EGuestOutcome UReputationLibrary::ToGuestOutcome(EPotionResult Result)
{
	switch (Result)
	{
	case EPotionResult::Human_Healed:
	case EPotionResult::Demon_GivenGoodPotion:
		return EGuestOutcome::Healed;

	case EPotionResult::Human_NotHealed:
	case EPotionResult::Demon_NotPoisoned:
		return EGuestOutcome::NotHealed;

	case EPotionResult::Human_Poisoned:
	case EPotionResult::Demon_Poisoned:
		return EGuestOutcome::Poisoned;
	}

	return EGuestOutcome::NotHealed;
}

EGuestFraction UReputationLibrary::GetRealFraction(EGuestFraction DisplayFraction, bool bIsDemon)
{
	return bIsDemon ? EGuestFraction::Demon : DisplayFraction;
}

const FReputationDelta* UReputationLibrary::FindDelta(const UGameSettings* Settings, const UGuestData* Guest, EGuestFraction RealFraction, EGuestOutcome Outcome)
{
	if (Guest && Guest->bOverrideReputation)
	{
		if (const FReputationDelta* Override = Guest->OverrideReputationRules.ByOutcome.Find(Outcome)) { return Override; }
	}

	if (!Settings) { return nullptr; }

	const FFractionReputationRules* Rules = Settings->ReputationRules.Find(RealFraction);
	return Rules ? Rules->ByOutcome.Find(Outcome) : nullptr;
}

float UReputationLibrary::GetDeathChance(const UGameSettings* Settings, const UGuestData* Guest, EGuestOutcome Outcome)
{
	if (Guest && Guest->bOverrideDeathChance)
	{
		if (const float* Override = Guest->OverrideDeathChance.Find(Outcome)) { return *Override; }
	}

	const float* Global = Settings ? Settings->GuestDeathChance.Find(Outcome) : nullptr;
	return Global ? *Global : 0.f;
}

FGuestOutcomeResult UReputationLibrary::ApplyOutcome(const UGameSettings* Settings, const UGuestData* Guest, bool bIsDemon, EGuestOutcome Outcome,
	const TArray<FGuestReputationModifier>& Modifiers, FGuestMetrics& Metrics, TMap<EGuestFraction, int32>& Reputation)
{
	FGuestOutcomeResult Result;
	if (!Guest || !Settings) { return Result; }

	Metrics.Outcomes.Add(Outcome);

	const bool bDeathAllowed = Metrics.VisitCount > Guest->MinVisitsBeforeDeath;
	const float DeathChance = GetDeathChance(Settings, Guest, Outcome);
	if (bDeathAllowed && DeathChance > 0.0f && FMath::FRand() < DeathChance) { Metrics.GuestAlive = false; }

	Result.bGuestAlive = Metrics.GuestAlive;

	const EGuestFraction RealFraction = GetRealFraction(Metrics.GuestFraction, bIsDemon);

	const FReputationDelta* Delta = FindDelta(Settings, Guest, RealFraction, Outcome);
	if (!Delta) { return Result; }

	for (const FGuestReputationModifier& Modifier : Modifiers)
	{
		if (Modifier.TriggerOutcome == Outcome)
		{
			Result.AppliedMultiplier *= Modifier.Multiplier;
		}
	}

	for (const TPair<EGuestFraction, int32>& Pair : Delta->Reaction)
	{
		const int32 Scaled = FMath::RoundToInt(Pair.Value * Result.AppliedMultiplier);
		if (Scaled == 0) { continue; }

		int32& Value = Reputation.FindOrAdd(Pair.Key);
		Value += Scaled;

		FReputationChange& Change = Result.ReputationChanges.AddDefaulted_GetRef();
		Change.Fraction = Pair.Key;
		Change.Delta = Scaled;
		Change.NewValue = Value;
	}

	return Result;
}

void UReputationLibrary::LogClientArrival(int32 ClientIndex, const FClient& Client)
{
	TArray<FString> Symptoms;
	for (const FName& Symptom : Client.Symptoms)
	{
		Symptoms.Add(Symptom.ToString());
	}

	const FString SymptomsString = Symptoms.IsEmpty() ? TEXT("No symptoms") : FString::Join(Symptoms, TEXT(", "));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Green, FString::Printf(TEXT("Client %d: %s"), ClientIndex, *SymptomsString));
	}

	UE_LOG(LogTemp, Warning, TEXT("Client %d symptoms: %s"), ClientIndex, *SymptomsString);
}
