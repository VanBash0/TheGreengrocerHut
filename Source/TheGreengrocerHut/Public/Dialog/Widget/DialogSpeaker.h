#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "DialogSpeaker.generated.h"

class UButton;
class UImage;
class UPaperSprite;
class UPanelWidget;
class UDoorDialogState;

UCLASS(BlueprintType, Blueprintable)
class THEGREENGROCERHUT_API UDialogSpeaker : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void SetGuest(UDoorDialogState* InState, int32 InGuestIndex);

	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void NotifyLeaveFinished();

	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void ReleaseGuest();

	UFUNCTION(BlueprintPure, Category = "Dialog")
	int32 GetGuestIndex() const { return GuestIndex; }

	UFUNCTION(BlueprintPure, Category = "Dialog")
	bool IsLeaving() const { return bLeaving; }

	UFUNCTION(BlueprintPure, Category = "Dialog")
	bool IsSwapping() const { return bSwapping; }

protected:
	virtual void NativeOnInitialized() override;

	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialog")
	void OnGuestAssigned(int32 InGuestIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialog")
	void OnNameUpdated(const FText& InNameText, bool bIsKnewName);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialog")
	void OnRealese();

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialog")
	void OnLeave();

private:
	UFUNCTION()
	void HandleClicked();

	UFUNCTION()
	void HandleDialogUpdated(bool bDialogEnded);

	UFUNCTION()
	void HandleGuestLeaving(int32 InGuestIndex);

	UFUNCTION()
	void HandleGuestDataChanged(FName GuestID);

	void Unbind();

	void ApplySprite(UPaperSprite* Sprite);

	void FinishSwap();

	void UpdateButtonStyle(UPaperSprite* Sprite);

	void ReleaseSprites();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bFollowActiveSpeaker = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bButtonClickable = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bShowSpeakerName = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (Categories = "DialogEmotion"))
	FGameplayTag DefaultEmotion;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (ClampMin = "0.01"))
	float SwapAnimSpeed = 4.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	FName CurrentGuestID;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> View_1;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> View_2;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPanelWidget> NamePanel;

private:
	UPROPERTY()
	TObjectPtr<UDoorDialogState> State;

	UPROPERTY()
	TObjectPtr<UImage> CurrentView;

	UPROPERTY()
	TObjectPtr<UImage> LastView;

	UPROPERTY()
	TObjectPtr<UPaperSprite> CurrentSprite;

	UPROPERTY()
	TObjectPtr<UPaperSprite> PendingSprite;

	int32 GuestIndex = INDEX_NONE;

	float SwapProgress = 0.0f;

	bool bSwapping = false;

	bool bLeaving = false;
};
