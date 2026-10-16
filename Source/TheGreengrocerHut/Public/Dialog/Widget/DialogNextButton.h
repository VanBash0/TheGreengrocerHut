#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "DialogNextButton.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNextClicked);

UCLASS(BlueprintType, Blueprintable)
class THEGREENGROCERHUT_API UDialogNextButton : public UUserWidget
{
	GENERATED_BODY()

public:
	UDialogNextButton(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void TakeFocus();

	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void SetShown(bool bInShown);

	UFUNCTION(BlueprintPure, Category = "Dialog")
	bool IsShown() const { return bShown; }

public:
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnNextClicked OnNextClicked;

protected:
	virtual void NativeOnInitialized() override;

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialog")
	void OnShown();

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialog")
	void OnHidden();

private:
	UFUNCTION()
	void HandleClicked();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	TArray<FKey> ContinueKeys;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> NextButton;

private:
	bool bShown = false;
};
