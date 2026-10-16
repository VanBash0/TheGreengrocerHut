#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "DialogSpeaker.generated.h"

class UButton;
class UImage;
class UPaperSprite;
class UDoorDialogState;

// Logic only: follows its guest in the state, crossfades the sprite, keeps the button style in sync.
// Show / Hide and the leave animation stay in BP
UCLASS(BlueprintType, Blueprintable)
class THEGREENGROCERHUT_API UDialogSpeaker : public UUserWidget
{
	GENERATED_BODY()

public:
	// The window calls it right after creating the speaker (replaces setting State / GuestIndex)
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void SetGuest(UDoorDialogState* InState, int32 InGuestIndex);

	// Call it from the leave animation end
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void NotifyLeaveFinished();

	// Frees the slot (replaces Realese): unsubscribes, forgets the guest and clears the picture
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

	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// Called at the end of SetGuest (the old SetupGuest place): call Show here
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialog")
	void OnGuestAssigned(int32 InGuestIndex);

	// Called at the end of ReleaseGuest (the old Realese): call Hide here
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialog")
	void OnRealese();

	// Play the leave animation, then call NotifyLeaveFinished
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialog")
	void OnLeave();

private:
	UFUNCTION()
	void HandleClicked();

	UFUNCTION()
	void HandleDialogUpdated(bool bDialogEnded);

	UFUNCTION()
	void HandleGuestLeaving(int32 InGuestIndex);

	void Unbind();

	void ApplySprite(UPaperSprite* Sprite);

	void FinishSwap();

	void UpdateButtonStyle(UPaperSprite* Sprite);

	// Drops every reference to the sprites (images, button style, cached pointers) so the guest can really be unloaded
	void ReleaseSprites();

protected:
	// true = shows the sprite of the active speaker while this guest is the selected one
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bFollowActiveSpeaker = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bButtonClickable = true;

	// Emotion shown when the guest is assigned, before the dialogue says anything (old SetupGuest: DialogEmotion.Idel)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (Categories = "DialogEmotion"))
	FGameplayTag DefaultEmotion;

	// Crossfade speed, 1 = one second
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (ClampMin = "0.01"))
	float SwapAnimSpeed = 4.0f;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> View_1;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> View_2;

private:
	UPROPERTY()
	TObjectPtr<UDoorDialogState> State;

	// The image that shows the sprite now / the one the next sprite fades in on
	UPROPERTY()
	TObjectPtr<UImage> CurrentView;

	UPROPERTY()
	TObjectPtr<UImage> LastView;

	UPROPERTY()
	TObjectPtr<UPaperSprite> CurrentSprite;

	// The sprite the running crossfade leads to
	UPROPERTY()
	TObjectPtr<UPaperSprite> PendingSprite;

	int32 GuestIndex = INDEX_NONE;

	float SwapProgress = 0.0f;

	bool bSwapping = false;

	bool bLeaving = false;
};
