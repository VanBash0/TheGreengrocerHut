#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DlgDialogueParticipant.h"
#include "DoorPlayerParticipant.generated.h"


UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class THEGREENGROCERHUT_API UDoorPlayerParticipant : public UActorComponent, public IDlgDialogueParticipant
{
	GENERATED_BODY()

public:
	UDoorPlayerParticipant();

public:
	virtual FName GetParticipantName_Implementation() const override;
};
