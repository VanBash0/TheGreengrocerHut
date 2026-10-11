#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClientStruct.h"
#include "DoorWindowManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDoorWindowReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDoorWindowClose);

class UDoorWindowState;
class UDoorExaminationState;
class UDoorDialogState;

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class THEGREENGROCERHUT_API UDoorWindowManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDoorWindowManager();

	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Manage")
	void Initialize();

	UFUNCTION(BlueprintCallable, Category = "Manage")
	void Reset();

public:
	UFUNCTION(BlueprintCallable, Category = "State")
	void GoToDialog();

	UFUNCTION(BlueprintCallable, Category = "State")
	void GoToExamination();

public:
	UFUNCTION(BlueprintCallable, Category = "Manage")
	void Show();

	UFUNCTION(BlueprintCallable, Category = "Manage")
	void Hide();

protected:
	UFUNCTION(BlueprintCallable, Category = "State")
	void SwitchState(UDoorWindowState* NewState);

protected:
	UFUNCTION()
	void OnStateReadyHandler(UDoorWindowState* InState);

public:
	UFUNCTION(BlueprintCallable, Category = "Manage")
	void RequestClose() { if (bShown) { OnWindowClose.Broadcast(); } }

	UFUNCTION(BlueprintPure, Category = "Manage")
	bool IsStatesReady() const { return bStatesReady; }

public:
	UPROPERTY(BlueprintAssignable, Category = "Manage")
	FOnDoorWindowReady OnReady;

	UPROPERTY(BlueprintAssignable, Category = "Manage")
	FOnDoorWindowClose OnWindowClose;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "State")
	TObjectPtr<UDoorExaminationState> ExaminationState;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	TObjectPtr<UDoorDialogState> DialogState;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	TObjectPtr<UDoorWindowState> CurrentState;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Manage")
	bool bStatesReady = false;

	UPROPERTY(BlueprintReadOnly, Category = "Manage")
	bool bShown = false;
};