#include "Dialog/Widget/DialogOptionButton.h"

#include "Components/Button.h"
#include "Dialog/GuestConditionCustom.h"

void UDialogOptionButton::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (OptionButton)
	{
		OptionButton->OnClicked.AddDynamic(this, &UDialogOptionButton::HandleClicked);
	}
}

bool UDialogOptionButton::SetEdge(const FDlgEdgeData& InEdge, const UDlgContext* Context, int32 InOptionIndex)
{
	bActive = InEdge.IsSatisfied();
	OptionIndex = bActive ? InOptionIndex : INDEX_NONE;
	OptionText = InEdge.GetEdge().GetText();
	ReasonText = Context ? BuildReason(InEdge.GetEdge(), *Context) : FText::GetEmpty();

	if (OptionButton)
	{
		// Disabled button has no hover / pressed look and gives no click feedback
		OptionButton->SetIsEnabled(bActive);
	}

	OnOptionUpdated(OptionText, bActive, ReasonText);
	return bActive;
}

FText UDialogOptionButton::BuildReason(const FDlgEdge& Edge, const UDlgContext& Context) const
{
	TArray<FText> Reasons;

	for (const FDlgCondition& Condition : Edge.Conditions)
	{
		if (Condition.ConditionType != EDlgConditionType::Custom) { continue; }

		const UGuestConditionCustom* Custom = Cast<UGuestConditionCustom>(Condition.CustomCondition);
		if (!Custom) { continue; }

		const FName ParticipantName = Condition.ParticipantName.IsNone() ? Context.GetActiveNodeParticipantName() : Condition.ParticipantName;
		const UObject* Participant = Context.GetParticipant(ParticipantName);

		const bool bMet = Condition.IsConditionMet(Context, Participant);

		const FText Reason = Custom->GetReason(&Context, Participant, bMet);
		if (!Reason.IsEmpty())
		{
			Reasons.Add(Reason);
		}
	}

	return FText::Join(FText::FromString(TEXT("\n")), Reasons);
}

void UDialogOptionButton::HandleClicked()
{
	if (!bActive || OptionIndex == INDEX_NONE) { return; }

	OnOptionClicked.Broadcast(OptionIndex);
}
