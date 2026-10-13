#include "Dialog/State/DoorDialogState.h"
#include "GameFramework/Actor.h"
#include "Engine/AssetManager.h"
#include "DlgContext.h"
#include "DlgManager.h"
#include "Dialog/State/DoorPlayerParticipant.h"
#include "Dialog/GuestEmotionPicker.h"
#include "Dialog/GuestDialogStructs.h"
#include <GameLoop.h>

namespace
{
	void ReleaseStreamHandle(TSharedPtr<FStreamableHandle>& Handle)
	{
		if (!Handle.IsValid()) { return; }

		if (Handle->IsLoadingInProgress())
		{
			Handle->CancelHandle();
		}
		else
		{
			Handle->ReleaseHandle();
		}
		Handle.Reset();
	}
}

void UDoorDialogState::BeginPlay()
{
	PlayerParticipant = GetOwner()->GetComponentByClass<UDoorPlayerParticipant>();
	if (!PlayerParticipant)
	{
		UE_LOG(LogTemp, Error, TEXT("[UDoorDialogState] %s: no UDoorPlayerParticipant component found on actor '%s'. "
			"Add a component UDoorPlayerParticipant."), *GetName(), *GetNameSafe(GetOwner()));
	}

	UDlgManager::GetDialogueHistory();

	Super::BeginPlay();
}

void UDoorDialogState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ReleaseAll();

	Super::EndPlay(EndPlayReason);
}

void UDoorDialogState::Initialize_Implementation(const FClient& InClient)
{
	Super::Initialize_Implementation(InClient);

	PendingGuestRefs = InClient.Guests;

	TArray<FSoftObjectPath> Paths;
	for (const TSoftObjectPtr<UGuestData>& Guest : PendingGuestRefs)
	{
		if (!Guest.IsNull())
		{
			Paths.AddUnique(Guest.ToSoftObjectPath());
		}
	}

	if (Paths.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[UDoorDialogState] %s: the client has no guests to load."), *GetName());
		PendingGuestRefs.Reset();
		NotifyStateReady();
		return;
	}

	GuestsHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(Paths,
		FStreamableDelegate::CreateWeakLambda(this, [this, Generation = LoadGeneration]()
		{
			if (Generation == LoadGeneration)
			{
				OnGuestsLoaded();
			}
		}));
}

void UDoorDialogState::Reset_Implementation()
{
	Super::Reset_Implementation();

	ReleaseAll();
}

void UDoorDialogState::Enter()
{
	Super::Enter();

	if (CurrentGuestIndex != INDEX_NONE)
	{
		OnGuestSelected.Broadcast(CurrentGuestIndex);
		if (UDlgContext* Context = GetCurrentContext())
		{
			OnDialogUpdated.Broadcast(Context->HasDialogueEnded());
		}
		return;
	}

	int32 OnlyGuest = INDEX_NONE;
	int32 Alive = 0;
	for (int32 i = 0; i < Guests.Num(); ++i)
	{
		if (!Guests[i].bLeft && !Guests[i].bLeaving) { OnlyGuest = i; ++Alive; }
	}

	if (Alive == 1) { SelectGuest(OnlyGuest); }
	else { OnBackToGuestsList.Broadcast(); }
}

void UDoorDialogState::AcceptCurrentGuest()
{
	if (!Guests.IsValidIndex(CurrentGuestIndex)) { return; }

	for (int32 i = 0; i < Guests.Num(); ++i)
	{
		if (i != CurrentGuestIndex) { StartGuestLeave(i); }
	}
}

void UDoorDialogState::RejectCurrentGuest()
{
	if (bRejectPending) { return; }

	if (!CanReject() || !Guests.IsValidIndex(CurrentGuestIndex)) { return; }

	bRejectPending = true;

	UGameLoop* Loop = GetWorld()->GetSubsystem<UGameLoop>();
	if (!Loop) { return; }

	Loop->UpdateGuestOutcome(Guests[CurrentGuestIndex].GuestData, EGuestOutcome::Rejected, Guests[CurrentGuestIndex].Modifiers);
}

void UDoorDialogState::StartGuestLeave(int32 GuestIndex)
{
	if (!Guests.IsValidIndex(GuestIndex)) { return; }

	FGuestEntry& Entry = Guests[GuestIndex];
	if (Entry.bLeft || Entry.bLeaving) { return; }

	Entry.bLeaving = true;
	OnGuestLeaving.Broadcast(GuestIndex);
}

void UDoorDialogState::FinishGuestLeave(int32 GuestIndex)
{
	if (!Guests.IsValidIndex(GuestIndex)) { return; }
	if (!Guests[GuestIndex].bLeaving) { return; }

	const bool bWasCurrent = (GuestIndex == CurrentGuestIndex);
	if (bWasCurrent) { CurrentGuestIndex = INDEX_NONE; }

	ReleaseGuest(Guests[GuestIndex]);

	if (bWasCurrent) { OnBackToGuestsList.Broadcast(); }
}

UDlgContext* UDoorDialogState::GetCurrentContext() const
{
	return Guests.IsValidIndex(CurrentGuestIndex) ? Guests[CurrentGuestIndex].Context.Get() : nullptr;
}

void UDoorDialogState::AddReputationModifier(const FGuestReputationModifier& ReputationModifier)
{
	if (!Guests.IsValidIndex(CurrentGuestIndex)) { return; }

	Guests[CurrentGuestIndex].Modifiers.Add(ReputationModifier);
}

bool UDoorDialogState::CanReject() const
{
	int32 Count = 0;
	for (const FGuestEntry& Entry : Guests)
	{
		if (!Entry.bLeft && !Entry.bLeaving) { ++Count; }
	}
	return Count > 1;
}

FName UDoorDialogState::GetParticipantName_Implementation() const
{
	return FName(TEXT("Guest")); 
}

FText UDoorDialogState::GetParticipantDisplayName_Implementation(FName ActiveSpeaker) const
{
	return Guests.IsValidIndex(CurrentGuestIndex) && Guests[CurrentGuestIndex].GuestData
		? Guests[CurrentGuestIndex].GuestData->GuestName : FText::GetEmpty();
}

ETextGender UDoorDialogState::GetParticipantGender_Implementation() const
{
	return Guests.IsValidIndex(CurrentGuestIndex) && Guests[CurrentGuestIndex].GuestData
		? Guests[CurrentGuestIndex].GuestData->GuestGeneder : ETextGender::Neuter;
}

UPaperSprite* UDoorDialogState::GetActiveSpeakerSprite() const
{
	if (!Guests.IsValidIndex(CurrentGuestIndex)) { return nullptr; }

	const FGuestEntry& Guest = Guests[CurrentGuestIndex];
	if (!Guest.EmotionPicker || !Guest.Context) { return nullptr; }

	if (Guest.Context->GetActiveNodeParticipantName() != GetParticipantName_Implementation()) { return nullptr; }

	return Guest.EmotionPicker->GetEmotionSprite(Guest.Context->GetActiveNodeSpeakerState());
}

void UDoorDialogState::SelectGuest(int32 InGuestIndex)
{
	if (CurrentGuestIndex != INDEX_NONE) { return; }
	if (!Guests.IsValidIndex(InGuestIndex)) { return; }

	FGuestEntry& Entry = Guests[InGuestIndex];
	if (Entry.bLeft || Entry.bLeaving) { return; }

	CurrentGuestIndex = InGuestIndex;

	if (!Entry.Context)
	{
		if (!Entry.Dialogue || !PlayerParticipant)
		{
			UE_LOG(LogTemp, Error, TEXT("[UDoorDialogState] %s: can't start the dialogue for guest %d (Dialogue=%s, PlayerParticipant=%s)."),
				*GetName(), InGuestIndex, *GetNameSafe(Entry.Dialogue), *GetNameSafe(PlayerParticipant));
			CurrentGuestIndex = INDEX_NONE;
			return;
		}

		Entry.Context = UDlgManager::StartDialogue2(Entry.Dialogue, this, PlayerParticipant);
		if (!Entry.Context)
		{
			UE_LOG(LogTemp, Error, TEXT("[UDoorDialogState] %s: StartDialogue2 failed for guest %d, check the participant names."),
				*GetName(), InGuestIndex);
			CurrentGuestIndex = INDEX_NONE;
			return;
		}
	}

	OnGuestSelected.Broadcast(CurrentGuestIndex);
	OnDialogUpdated.Broadcast(Entry.Context->HasDialogueEnded());
}

void UDoorDialogState::BackToGuestsList()
{
	if (CurrentGuestIndex == INDEX_NONE) { return; }

	CurrentGuestIndex = INDEX_NONE;
	OnBackToGuestsList.Broadcast();
}

void UDoorDialogState::SelectDialogOption(int32 OptionIndex)
{
	UDlgContext* Context = GetCurrentContext();
	if (!Context) { return; }
	if (!Context->IsValidOptionIndex(OptionIndex)) { return; }

	const bool bContinues = Context->ChooseOption(OptionIndex);

	OnDialogUpdated.Broadcast(!bContinues);

	if (bRejectPending && !bContinues)
	{
		bRejectPending = false;
		StartGuestLeave(CurrentGuestIndex);
	}
}

FGuestMetrics UDoorDialogState::GetGuestRealMetrics(UGuestData* InGuestData) const
{
	UGameLoop* Loop = GetWorld()->GetSubsystem<UGameLoop>();
	if (!Loop) { return InGuestData->DefaultMetrics; }
	
	return Loop->GetGuestMetrics(InGuestData);
}

void UDoorDialogState::OnGuestsLoaded()
{
	for (const TSoftObjectPtr<UGuestData>& Soft : PendingGuestRefs)
	{
		UGuestData* Data = Soft.Get();
		if (!Data)
		{
			UE_LOG(LogTemp, Error, TEXT("[UDoorDialogState] %s: guest '%s' failed to load, skipped."), *GetName(), *Soft.ToString());
			continue;
		}

		if (UGameLoop* Loop = GetWorld()->GetSubsystem<UGameLoop>())
		{
			Loop->RegisterGuestVisit(Data);
		}

		FGuestEntry& Entry = Guests.AddDefaulted_GetRef();
		Entry.GuestData = Data;
		Entry.GuestMetrics = GetGuestRealMetrics(Data);

		Entry.EmotionPicker = NewObject<UGuestEmotionPicker>(this);
		Entry.EmotionPicker->OnLoaded.AddDynamic(this, &UDoorDialogState::OnPickerLoaded);
		++PendingLoads;

		if (!Data->GuestDialogue.IsNull()) { ++PendingLoads; }
	}
	PendingGuestRefs.Reset();

	if (Guests.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[UDoorDialogState] %s: no guest was loaded."), *GetName());
		NotifyStateReady();
		return;
	}

	for (int32 Index = 0; Index < Guests.Num(); ++Index)
	{
		FGuestEntry& Entry = Guests[Index];

		Entry.EmotionPicker->Initialize(Entry.GuestData);

		const TSoftObjectPtr<UDlgDialogue>& Dialogue = Entry.GuestData->GuestDialogue;
		if (!Dialogue.IsNull())
		{
			Entry.DialogueHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(Dialogue.ToSoftObjectPath(),
				FStreamableDelegate::CreateWeakLambda(this, [this, Index, Generation = LoadGeneration]()
				{
					if (Generation == LoadGeneration)
					{
						OnDialogueLoaded(Index);
					}
				}));
		}
	}
}

void UDoorDialogState::OnDialogueLoaded(int32 GuestIndex)
{
	if (Guests.IsValidIndex(GuestIndex))
	{
		FGuestEntry& Entry = Guests[GuestIndex];
		if (!Entry.bLeft && Entry.GuestData)
		{
			Entry.Dialogue = Entry.GuestData->GuestDialogue.Get();
		}
	}

	FinishLoadStep();
}

void UDoorDialogState::OnPickerLoaded()
{
	FinishLoadStep();
}

void UDoorDialogState::FinishLoadStep()
{
	if (IsStateReady() || PendingLoads <= 0) { return; }

	if (--PendingLoads == 0) { NotifyStateReady(); }
}

void UDoorDialogState::ReleaseGuest(FGuestEntry& Entry)
{
	if (Entry.EmotionPicker)
	{
		Entry.EmotionPicker->OnLoaded.RemoveDynamic(this, &UDoorDialogState::OnPickerLoaded);
		Entry.EmotionPicker->Release();
		Entry.EmotionPicker = nullptr;
	}

	ReleaseStreamHandle(Entry.DialogueHandle);

	Entry.Context = nullptr;
	Entry.Dialogue = nullptr;
	Entry.GuestData = nullptr;
	Entry.bLeaving = false;
	Entry.bLeft = true;
}

void UDoorDialogState::ReleaseAll()
{
	++LoadGeneration;
	PendingLoads = 0;
	PendingGuestRefs.Reset();
	CurrentGuestIndex = INDEX_NONE;
	bRejectPending = false;

	for (FGuestEntry& Entry : Guests) { ReleaseGuest(Entry); }
	Guests.Empty();

	ReleaseStreamHandle(GuestsHandle);
}