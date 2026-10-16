#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DlgContext.h"
#include "DialogOptionButton.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogOptionClicked, int32, OptionIndex);

UCLASS(BlueprintType, Blueprintable)
class THEGREENGROCERHUT_API UDialogOptionButton : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	bool SetEdge(const FDlgEdgeData& InEdge, const UDlgContext* Context, int32 InOptionIndex);

	UFUNCTION(BlueprintPure, Category = "Dialog")
	bool IsButtonActive() const { return bActive; }

	UFUNCTION(BlueprintPure, Category = "Dialog")
	FText GetOptionText() const { return OptionText; }

	UFUNCTION(BlueprintPure, Category = "Dialog")
	FText GetReasonText() const { return ReasonText; }

	UFUNCTION(BlueprintPure, Category = "Dialog")
	int32 GetOptionIndex() const { return OptionIndex; }

public:
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnDialogOptionClicked OnOptionClicked;

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialog")
	void OnOptionUpdated(const FText& InOptionText, bool bInActive, const FText& InReasonText);

private:
	UFUNCTION()
	void HandleClicked();

	FText BuildReason(const FDlgEdge& Edge, const UDlgContext& Context) const;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> OptionButton;

private:
	FText OptionText;
	FText ReasonText;
	int32 OptionIndex = INDEX_NONE;
	bool bActive = false;
};
