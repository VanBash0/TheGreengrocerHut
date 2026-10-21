#include "Dialog/GuestLibrary.h"

#include "GameSettings.h"

namespace
{
	TArray<FString> BuildParts(const TArray<FText>& Parts, const TArray<int32>& Unlocked, FName FakeStyle)
	{
		TArray<FString> Result;
		Result.Reserve(Parts.Num());

		for (int32 i = 0; i < Parts.Num(); ++i)
		{
			FString Text = Parts[i].ToString();

			if (!Unlocked.Contains(i))
			{
				Text = Text.Left(Text.Len() / 2);

				if (!FakeStyle.IsNone())
				{
					Text = FString::Printf(TEXT("<%s>%s</>"), *FakeStyle.ToString(), *Text);
				}
			}

			Result.Add(Text);
		}

		return Result;
	}
}

EGuestOutcome UGuestLibrary::ToGuestOutcome(EPotionResult Result)
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

EGuestFraction UGuestLibrary::GetRealFraction(EGuestFraction DisplayFraction, bool bIsDemon)
{
	return bIsDemon ? EGuestFraction::Demon : DisplayFraction;
}

const FReputationDelta* UGuestLibrary::FindDelta(const UGameSettings* Settings, const UGuestData* Guest, EGuestFraction RealFraction, EGuestOutcome Outcome)
{
	if (Guest && Guest->bOverrideReputation)
	{
		if (const FReputationDelta* Override = Guest->OverrideReputationRules.ByOutcome.Find(Outcome)) { return Override; }
	}

	if (!Settings) { return nullptr; }

	const FFractionReputationRules* Rules = Settings->ReputationRules.Find(RealFraction);
	return Rules ? Rules->ByOutcome.Find(Outcome) : nullptr;
}

float UGuestLibrary::GetDeathChance(const UGameSettings* Settings, const UGuestData* Guest, EGuestOutcome Outcome)
{
	if (Guest && Guest->bOverrideDeathChance)
	{
		if (const float* Override = Guest->OverrideDeathChance.Find(Outcome)) { return *Override; }
	}

	const float* Global = Settings ? Settings->GuestDeathChance.Find(Outcome) : nullptr;
	return Global ? *Global : 0.f;
}

FGuestOutcomeResult UGuestLibrary::ApplyOutcome(const UGameSettings* Settings, const UGuestData* Guest, bool bIsDemon, EGuestOutcome Outcome,
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

void UGuestLibrary::LogClientArrival(int32 ClientIndex, const FClient& Client)
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

FText UGuestLibrary::GetDescription(const FGuestInfo& Info, FName FakeStyle, const FString& Separator)
{
	const TArray<FString> Parts = BuildParts(Info.DescriptionParts, Info.Metrics.UnlockedDescription, FakeStyle);
	return FText::FromString(FString::Join(Parts, *Separator));
}

FText UGuestLibrary::GetNotes(const FGuestInfo& Info, FName FakeStyle, const FString& Bullet)
{
	FString Result;

	for (const FString& Part : BuildParts(Info.NotesParts, Info.Metrics.UnlockedNotes, FakeStyle))
	{
		Result += Bullet + Part + TEXT("\n");
	}

	return FText::FromString(Result);
}
