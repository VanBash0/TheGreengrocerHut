#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Dialog/GuestDialogStructs.h"
#include "ClientStruct.h"
#include "ReputationLibrary.generated.h"

class UGameSettings;

UCLASS()
class THEGREENGROCERHUT_API UReputationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Reputation")
	static EGuestOutcome ToGuestOutcome(EPotionResult Result);

	UFUNCTION(BlueprintPure, Category = "Reputation")
	static EGuestFraction GetRealFraction(EGuestFraction DisplayFraction, bool bIsDemon);

	static const FReputationDelta* FindDelta(const UGameSettings* Settings, const UGuestData* Guest, EGuestFraction RealFraction, EGuestOutcome Outcome);

	static float GetDeathChance(const UGameSettings* Settings, const UGuestData* Guest, EGuestOutcome Outcome);

	static FGuestOutcomeResult ApplyOutcome(const UGameSettings* Settings, const UGuestData* Guest, bool bIsDemon, EGuestOutcome Outcome,
		const TArray<FGuestReputationModifier>& Modifiers, FGuestMetrics& Metrics, TMap<EGuestFraction, int32>& Reputation);

	static void LogClientArrival(int32 ClientIndex, const FClient& Client);
};
