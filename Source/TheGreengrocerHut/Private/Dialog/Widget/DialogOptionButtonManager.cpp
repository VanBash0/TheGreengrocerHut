#include "Dialog/Widget/DialogOptionButtonManager.h"

#include "Components/PanelWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Dialog/Widget/DialogOptionButton.h"
#include "Dialog/Widget/DialogNextButton.h"
#include "Dialog/GuestConditionCustom.h"
#include "DlgContext.h"

void UDialogOptionButtonManager::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	CollectButtons();

	if (NextButton)
	{
		NextButton->OnNextClicked.AddDynamic(this, &UDialogOptionButtonManager::ChooseNext);
	}
}

void UDialogOptionButtonManager::CollectButtons()
{
	Pool.Reset();

	if (!ButtonsContainer) { return; }

	for (UWidget* Child : ButtonsContainer->GetAllChildren())
	{
		if (UDialogOptionButton* Button = Cast<UDialogOptionButton>(Child))
		{
			Button->OnOptionClicked.AddDynamic(this, &UDialogOptionButtonManager::HandleOptionClicked);
			Button->SetVisibility(ESlateVisibility::Collapsed);
			Pool.Add(Button);
		}
	}

	if (Pool.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[UDialogOptionButtonManager] %s: ButtonsContainer has no DialogOptionButton inside."), *GetName());
	}
}

bool UDialogOptionButtonManager::HasReasonCondition(const FDlgEdge& Edge)
{
	for (const FDlgCondition& Condition : Edge.Conditions)
	{
		if (Condition.ConditionType == EDlgConditionType::Custom && Cast<UGuestConditionCustom>(Condition.CustomCondition))
		{
			return true;
		}
	}

	return false;
}

void UDialogOptionButtonManager::UpdateOptions(const UDlgContext* Context)
{
	if (!Context) { return; }

	// A locked option is shown only if it can explain itself (has a GuestConditionCustom), otherwise it is hidden as usual
	TArray<const FDlgEdgeData*> Edges;
	for (const FDlgEdgeData& Edge : Context->GetAllOptionsArray())
	{
		if (Edge.IsSatisfied() || HasReasonCondition(Edge.GetEdge()))
		{
			Edges.Add(&Edge);
		}
	}

	if (Edges.Num() > Pool.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UDialogOptionButtonManager] %s: the node has %d options, but the container has only %d buttons. Extra options are not shown."),
			*GetName(), Edges.Num(), Pool.Num());
	}

	VisibleCount = FMath::Min(Edges.Num(), Pool.Num());
	ActiveCount = 0;
	FText LastActiveText;

	for (int32 i = 0; i < VisibleCount; ++i)
	{
		UDialogOptionButton* Button = Pool[i];
		if (!Button) { continue; }

		Button->SetVisibility(ESlateVisibility::Visible);

		if (Button->SetEdge(*Edges[i], Context, ActiveCount))
		{
			++ActiveCount;
			LastActiveText = Button->GetOptionText();
		}
	}

	for (int32 i = VisibleCount; i < Pool.Num(); ++i)
	{
		if (Pool[i])
		{
			Pool[i]->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	const FString LastText = LastActiveText.ToString().TrimStartAndEnd();
	const bool bContinueText = LastText.IsEmpty() || LastText.Equals(NextOptionText.TrimStartAndEnd(), ESearchCase::IgnoreCase);

	bNextMode = Edges.Num() == 1 && ActiveCount == 1 && bContinueText;

	if (ModeSwitcher)
	{
		ModeSwitcher->SetActiveWidgetIndex(bNextMode ? NextPageIndex : OptionsPageIndex);
	}

	if (NextButton)
	{
		NextButton->SetShown(bNextMode);

		if (bNextMode)
		{
			NextButton->TakeFocus();
		}
	}

	OnOptionsUpdated(bNextMode, VisibleCount, ActiveCount);
}

void UDialogOptionButtonManager::ChooseNext()
{
	if (!bNextMode) { return; }

	OnOptionSelected.Broadcast(0);
}

void UDialogOptionButtonManager::HandleOptionClicked(int32 OptionIndex)
{
	OnOptionSelected.Broadcast(OptionIndex);
}
