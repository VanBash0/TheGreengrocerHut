#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TimerHandle.h"
#include "BookViewer.generated.h"

class ABook;
class APlayerController;

class UCameraComponent;
class UPointLightComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFocusedBookChanged, ABook*, PreviouseBook, ABook*, NewBook, int32, NewIndex);

UENUM(BlueprintType)
enum class EBookViewerState : uint8
{
	Inactive,
	Entering,
	Active,
	Exiting
};

USTRUCT(BlueprintType)
struct FBookSlot
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<ABook> Book;

	// ѕоза камеры дл€ этой книги ќ“Ќќ—»“≈Ћ№Ќќ viewer'а (гизмо во вьюпорте).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MakeEditWidget = true))
	FTransform ViewTransfrom;
};

UCLASS(BlueprintType)
class THEGREENGROCERHUT_API ABookViewer : public AActor
{
	GENERATED_BODY()

public:
	ABookViewer();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category = "EnterExit")
	bool TryEnter(ABook* StartBook);

	UFUNCTION(BlueprintImplementableEvent, Category = "EnterExit")
	void OnStartEntering();

	UFUNCTION(BlueprintCallable, Category = "EnterExit")
	bool TryExit();

	UFUNCTION(BlueprintImplementableEvent, Category = "EnterExit")
	void OnFinishExiting();

protected:
	UFUNCTION(BlueprintCallable, Category = "InitializeState")
	void SnapCameraToFocus();

protected:
	UFUNCTION(BlueprintCallable, Category = "StateControl")
	void SetFocusedIndex(int32 NewIndex);

protected:
	UFUNCTION(BlueprintCallable, Category = "Helpers")
	int32 FindBookSlotIndexByBookActor(const ABook* Book) const;

	UFUNCTION(BlueprintCallable, Category = "Helpers")
	ABook* GetFocusedBook() const;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USceneComponent> CameraRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UPointLightComponent> Light;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Bind")
	TArray<FBookSlot> BookSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Blend", meta = (ClampMin = "0.0"))
	float EnterBlendTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Blend", meta = (ClampMin = "0.0"))
	float ExitBlendTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Switching", meta = (ClampMin = "0.0"))
	float CameraSwitchSpeed = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Switching", meta = (ClampMin = "0.0"))
	float SwitchHysteresis = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Switching")
	bool bRequireHoverBeforeSwitch = true;

public:
	UPROPERTY(BlueprintAssignable, Category = "Runtime|State")
	FOnFocusedBookChanged OnFocusedBookChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime|State")
	EBookViewerState CurrentState = EBookViewerState::Inactive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime|State")
	int32 FocusedIndex = INDEX_NONE;

private:
	void FinishEnter();
	void FinishExit();

	void UpdateFocusFromCursor();
	void UpdateCameraRig(float DeltaTime);

	bool GetBookScreenRect(const ABook* Book, APlayerController* PC, FBox2D& OutRect) const;

	static float DistanceToRect(const FBox2D& Rect, const FVector2D& Point);

private:
	TWeakObjectPtr<APlayerController> ActivePC;
	TWeakObjectPtr<AActor> PreviousViewTarget;

	FTimerHandle StateTimerHandle;

	bool bPendingExit = false;

	bool bSwitchArmed = false;

	FVector RigLocation = FVector::ZeroVector;
	FQuat RigRotation = FQuat::Identity;
};