#include "Dialog/State/DoorWindowState.h"

UDoorWindowState::UDoorWindowState()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDoorWindowState::Enter()
{
	OnEnter();
}

void UDoorWindowState::Exit()
{
	OnExit();
}

