#include "Dialog/State/DoorExaminationState.h"
#include "Kismet/GameplayStatics.h"
#include "Symptom/SymptomViewer.h"

void UDoorExaminationState::BeginPlay()
{
	SymptomViewer = Cast<ASymptomViewer>(UGameplayStatics::GetActorOfClass(GetWorld(), ASymptomViewer::StaticClass()));

	if (SymptomViewer)
	{
		SymptomViewer->OnRenderComplete.AddDynamic(this, &UDoorExaminationState::NotifyStateReady);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[DoorExaminationState] SymptomViewer not found: place one in the level."));
	}

	Super::BeginPlay();
}

void UDoorExaminationState::Initialize_Implementation(const FClient& InClient)
{
	Super::Initialize_Implementation(InClient);

	if (!SymptomViewer) { return; }

	SymptomViewer->SetNewSymptoms(InClient);
}

void UDoorExaminationState::Reset_Implementation()
{
	Super::Reset_Implementation();

	if (!SymptomViewer) { return; }

	SymptomViewer->Reset();
}
