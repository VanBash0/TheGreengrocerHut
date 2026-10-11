#include "Dialog/State/DoorWindowManager.h"
#include "GameFramework/Actor.h"
#include "GameLoop.h"
#include "Dialog/State/DoorExaminationState.h"
#include "Dialog/State/DoorDialogState.h"

UDoorWindowManager::UDoorWindowManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDoorWindowManager::BeginPlay()
{
	Super::BeginPlay();

	const AActor* Owner = GetOwner();

	ExaminationState = Owner->GetComponentByClass<UDoorExaminationState>();
	if (ExaminationState)
	{
		ExaminationState->SetManager(this);
		ExaminationState->OnStateReady.AddDynamic(this, &UDoorWindowManager::OnStateReadyHandler);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[DoorWindowManager] %s: no UDoorExaminationState component found on actor '%s'. "
			"Add a component derived from UDoorExaminationState (BPC_Examination must be reparented to it)."),
			*GetName(), *GetNameSafe(Owner));
	}

	DialogState = Owner->GetComponentByClass<UDoorDialogState>();
	if (DialogState)
	{
		DialogState->SetManager(this);
		DialogState->OnStateReady.AddDynamic(this, &UDoorWindowManager::OnStateReadyHandler);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[DoorWindowManager] %s: no UDoorDialogState component found on actor '%s'. "
			"Add a component derived from UDoorDialogState (BPC_Dialog must be reparented to it)."),
			*GetName(), *GetNameSafe(Owner));
	}

	if (!DialogState || !ExaminationState) { return; }
}

void UDoorWindowManager::Initialize()
{
	if (!DialogState || !ExaminationState)
	{
		UE_LOG(LogTemp, Error, TEXT("[DoorWindowManager] Initialize: states are missing, check BeginPlay log."));
		return;
	}

	ensureMsgf(!bStatesReady && !bShown, TEXT("DoorWindowManager::Initialize called without Reset()"));

	UGameLoop* GameLoop = GetWorld()->GetSubsystem<UGameLoop>();
	if (!GameLoop)
	{
		UE_LOG(LogTemp, Error, TEXT("[DoorWindowManager] GameLoop subsystem not found."));
		return;
	}

	FClient Client;
	GameLoop->GetCurrentClient(Client);

	DialogState->Initialize(Client);
	ExaminationState->Initialize(Client);

	OnStateReadyHandler(DialogState);
}

void UDoorWindowManager::Reset()
{
	Hide();
	CurrentState = DialogState;
	bStatesReady = false;

	if (DialogState) { DialogState->Reset(); }
	if (ExaminationState) { ExaminationState->Reset(); }
}

void UDoorWindowManager::Show()
{
	if (!IsStatesReady() || bShown) { return; }
	if (!CurrentState) { CurrentState = DialogState; }
	bShown = true;
	CurrentState->Enter();
}

void UDoorWindowManager::Hide()
{
	if (!bShown) { return; }
	bShown = false;
	if (CurrentState) { CurrentState->Exit(); }
}

void UDoorWindowManager::GoToDialog() { SwitchState(DialogState); }

void UDoorWindowManager::GoToExamination() { SwitchState(ExaminationState); }

void UDoorWindowManager::SwitchState(UDoorWindowState* NewState)
{
	if (!NewState || NewState == CurrentState) { return; }
	if (bShown && CurrentState) { CurrentState->Exit(); }
	CurrentState = NewState;
	if (bShown) { CurrentState->Enter(); }
}

void UDoorWindowManager::OnStateReadyHandler(UDoorWindowState* InState)
{
	if (IsStatesReady()) { return; }
	if (!DialogState || !ExaminationState) { return; }
	if (!ExaminationState->IsStateReady() || !DialogState->IsStateReady()) { return; }

	bStatesReady = true;
	OnReady.Broadcast();
}
