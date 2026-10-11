#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClientStruct.h"
#include "DoorWindowState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateReady, UDoorWindowState*, State);

class UDoorWindowManager;

UCLASS(Abstract, Blueprintable)
class THEGREENGROCERHUT_API UDoorWindowState : public UActorComponent
{
	GENERATED_BODY()

public:
	UDoorWindowState();

public:
	UFUNCTION(BlueprintNativeEvent, Category = "State")
	void Initialize(const FClient& InClient);
	virtual void Initialize_Implementation(const FClient& InClient) {}

	UFUNCTION(BlueprintNativeEvent, Category = "State")
	void Reset();
	virtual void Reset_Implementation() { bStateReady = false; }

public:
	UFUNCTION(BlueprintPure, Category = "State")
	bool IsStateReady() const { return bStateReady; }

protected:
	UFUNCTION(BlueprintCallable, Category = "State")
	void NotifyStateReady()
	{
		if (bStateReady) { return; }
		bStateReady = true;
		OnStateReady.Broadcast(this);
	}

public:
	UFUNCTION(BlueprintPure, Category = "State")
	UDoorWindowManager* GetManager() const { return StateManager; }

	void SetManager(UDoorWindowManager* InManager) { StateManager = InManager; }

public:
	virtual void Enter();

	virtual void Exit();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "State")
	void OnEnter();

	UFUNCTION(BlueprintImplementableEvent, Category = "State")
	void OnExit();

public:
	UPROPERTY(BlueprintAssignable, Category = "State")
	FOnStateReady OnStateReady;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "State")
	TObjectPtr<UDoorWindowManager> StateManager;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bStateReady = false;
};
