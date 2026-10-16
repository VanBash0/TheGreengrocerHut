#include "Dialog/Widget/DialogNextButton.h"

#include "Components/Button.h"

UDialogNextButton::UDialogNextButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);

	ContinueKeys = { EKeys::SpaceBar, EKeys::Enter };
}

void UDialogNextButton::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (NextButton)
	{
		NextButton->OnClicked.AddDynamic(this, &UDialogNextButton::HandleClicked);
	}
}

void UDialogNextButton::TakeFocus()
{
	if (IsVisible())
	{
		SetKeyboardFocus();
	}
}

void UDialogNextButton::SetShown(bool bInShown)
{
	if (bShown == bInShown) { return; }

	bShown = bInShown;

	if (bShown)
	{
		OnShown();
	}
	else
	{
		OnHidden();
	}
}

FReply UDialogNextButton::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (ContinueKeys.Contains(InKeyEvent.GetKey()))
	{
		HandleClicked();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UDialogNextButton::HandleClicked()
{
	OnNextClicked.Broadcast();
}
