#pragma once

#include "CoreMinimal.h"
#include "Dialog/State/DoorWindowState.h"
#include "DlgDialogueParticipant.h"
#include "ClientStruct.h"
#include "Engine/StreamableManager.h"
#include "DoorDialogState.generated.h"

class UDoorPlayerParticipant;
class UGuestEmotionPicker;
class UDlgContext;
class UDlgDialogue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGuestListShown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGuestSelected, int32, NewGuestIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogUpdated, bool, bDialogEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGuestLeaving, int32, GuestIndex);

USTRUCT(BlueprintType)
struct FGuestEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UGuestData> GuestData;

	UPROPERTY(BlueprintReadOnly)
	FGuestMetrics GuestMetrics;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UDlgDialogue> Dialogue;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UDlgContext> Context;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UGuestEmotionPicker> EmotionPicker;

	UPROPERTY(BlueprintReadOnly)
	TArray<FGuestReputationModifier> Modifiers;

	UPROPERTY(BlueprintReadOnly)
	bool bLeft = false;

	UPROPERTY(BlueprintReadOnly)
	bool bLeaving = false;

	TSharedPtr<FStreamableHandle> DialogueHandle;
};

UCLASS(BlueprintType)
class THEGREENGROCERHUT_API UDoorDialogState : public UDoorWindowState, public IDlgDialogueParticipant
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Initialize_Implementation(const FClient& InClient) override;

	virtual void Reset_Implementation() override;

public:
	virtual void Enter() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Manage")
	void AcceptCurrentGuest();

	UFUNCTION(BlueprintCallable, Category = "Manage")
	void RejectCurrentGuest();

	UFUNCTION(BlueprintCallable, Category = "Manage")
	void StartGuestLeave(int32 GuestIndex);

	UFUNCTION(BlueprintCallable, Category = "Manage")
	void FinishGuestLeave(int32 GuestIndex);

	UFUNCTION(BlueprintPure, Category = "Manage")
	bool CanReject() const;

public:
	virtual FName GetParticipantName_Implementation() const override;

	virtual FText GetParticipantDisplayName_Implementation(FName ActiveSpeaker) const override;

	virtual ETextGender GetParticipantGender_Implementation() const override;

	UFUNCTION(BlueprintPure, Category = "Dialog")
	virtual UPaperSprite* GetActiveSpeakerSprite() const;

public:
	UFUNCTION(BlueprintPure, Category = "Manage")
	FGuestEntry GetCurrentGuest() const { return Guests.IsValidIndex(CurrentGuestIndex) ? Guests[CurrentGuestIndex] : FGuestEntry(); }

	UFUNCTION(BlueprintPure, Category = "Manage")
	UDlgContext* GetCurrentContext() const;

	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void AddReputationModifier(const FGuestReputationModifier& ReputationModifier);

	UFUNCTION(BlueprintPure, Category = "Manage")
	void GetGuests(TArray<FGuestEntry>& OutGuests) const { OutGuests = Guests; }

	UFUNCTION(BlueprintCallable, Category = "Manage")
	void SelectGuest(int32 InGuestIndex);

	UFUNCTION(BlueprintCallable, Category = "Manage")
	void BackToGuestsList();

public:
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void SelectDialogOption(int32 OptionIndex);

protected:
	void OnGuestsLoaded();

	void OnDialogueLoaded(int32 GuestIndex);

	UFUNCTION()
	void OnPickerLoaded();

	void FinishLoadStep();

protected:
	void ReleaseGuest(FGuestEntry& Entry);

	void ReleaseAll();

protected:
	FGuestMetrics GetGuestRealMetrics(UGuestData* InGuestData) const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	TObjectPtr<UDoorPlayerParticipant> PlayerParticipant;

public:
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnGuestListShown OnBackToGuestsList;

	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnGuestSelected OnGuestSelected;

	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnDialogUpdated OnDialogUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnGuestLeaving OnGuestLeaving;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Manage")
	TArray<FGuestEntry> Guests;

	UPROPERTY(BlueprintReadOnly, Category = "Manage")
	int32 CurrentGuestIndex = INDEX_NONE;

private:
	TArray<TSoftObjectPtr<UGuestData>> PendingGuestRefs;

	TSharedPtr<FStreamableHandle> GuestsHandle;

	int32 PendingLoads = 0;

	int32 LoadGeneration = 0;

private:
	bool bRejectPending = false;
};
