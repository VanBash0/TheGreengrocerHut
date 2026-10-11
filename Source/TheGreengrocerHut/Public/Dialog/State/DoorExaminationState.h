#pragma once

#include "CoreMinimal.h"
#include "Dialog/State/DoorWindowState.h"
#include "DoorExaminationState.generated.h"

class ASymptomViewer;

UCLASS(BlueprintType)
class THEGREENGROCERHUT_API UDoorExaminationState : public UDoorWindowState
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	
public:
	virtual void Initialize_Implementation(const FClient& InClient) override;

	virtual void Reset_Implementation() override;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	TObjectPtr<ASymptomViewer> SymptomViewer;
};
