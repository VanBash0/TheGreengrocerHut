#include "Dialog/State/DoorPlayerParticipant.h"

UDoorPlayerParticipant::UDoorPlayerParticipant()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FName UDoorPlayerParticipant::GetParticipantName_Implementation() const
{
	return FName(TEXT("Player"));
}