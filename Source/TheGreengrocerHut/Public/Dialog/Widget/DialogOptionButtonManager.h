#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogOptionButtonManager.generated.h"

class UDlgContext;
struct FDlgEdge;
class UDialogOptionButton;
class UDialogNextButton;
class UPanelWidget;
class UWidgetSwitcher;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManagerOptionSelected, int32, OptionIndex);

UCLASS(BlueprintType, Blueprintable)
class THEGREENGROCERHUT_API UDialogOptionButtonManager : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void UpdateOptions(const UDlgContext* Context);

	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void ChooseNext();

	UFUNCTION(BlueprintPure, Category = "Dialog")
	int32 GetActiveOptionsCount() const { return ActiveCount; }

	UFUNCTION(BlueprintPure, Category = "Dialog")
	int32 GetVisibleOptionsCount() const { return VisibleCount; }

	UFUNCTION(BlueprintPure, Category = "Dialog")
	bool IsNextMode() const { return bNextMode; }

public:
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnManagerOptionSelected OnOptionSelected;

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialog")
	void OnOptionsUpdated(bool bInNextMode, int32 InVisibleCount, int32 InActiveCount);

private:
	// Takes the buttons that already lie in ButtonsContainer (in the order of the container)
	void CollectButtons();

	// Does the edge have a condition that can give a reason (UGuestConditionCustom)
	static bool HasReasonCondition(const FDlgEdge& Edge);

	UFUNCTION()
	void HandleOptionClicked(int32 OptionIndex);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	int32 OptionsPageIndex = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	int32 NextPageIndex = 1;

	// A single available option with an empty text or with exactly this text is shown as the Next button
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	FString NextOptionText = TEXT("Next");

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> ModeSwitcher;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPanelWidget> ButtonsContainer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UDialogNextButton> NextButton;

private:
	UPROPERTY()
	TArray<TObjectPtr<UDialogOptionButton>> Pool;

	int32 VisibleCount = 0;
	int32 ActiveCount = 0;
	bool bNextMode = false;
};
