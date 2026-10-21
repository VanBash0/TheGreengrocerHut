#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Dialog/GuestDialogStructs.h"
#include "Dialog/GuestSubsystem.h"
#include "ClientStruct.h"
#include "GuestLibrary.generated.h"

class UGameSettings;

UCLASS()
class THEGREENGROCERHUT_API UGuestLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Guest|Reputation")
	static EGuestOutcome ToGuestOutcome(EPotionResult Result);

	UFUNCTION(BlueprintPure, Category = "Guest|Reputation")
	static EGuestFraction GetRealFraction(EGuestFraction DisplayFraction, bool bIsDemon);

	static const FReputationDelta* FindDelta(const UGameSettings* Settings, const UGuestData* Guest, EGuestFraction RealFraction, EGuestOutcome Outcome);

	static float GetDeathChance(const UGameSettings* Settings, const UGuestData* Guest, EGuestOutcome Outcome);

	static FGuestOutcomeResult ApplyOutcome(const UGameSettings* Settings, const UGuestData* Guest, bool bIsDemon, EGuestOutcome Outcome,
		const TArray<FGuestReputationModifier>& Modifiers, FGuestMetrics& Metrics, TMap<EGuestFraction, int32>& Reputation);

	static void LogClientArrival(int32 ClientIndex, const FClient& Client);

public:
	UFUNCTION(BlueprintPure, Category = "Guest|Book")
	static FText GetDescription(const FGuestInfo& Info, FName FakeStyle, const FString& Separator = TEXT(" "));

	UFUNCTION(BlueprintPure, Category = "Guest|Book")
	static FText GetNotes(const FGuestInfo& Info, FName FakeStyle, const FString& Bullet = TEXT("- "));
};
