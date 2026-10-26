#include "Dialog/Widget/DialogSpeaker.h"


#include "Components/PanelWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Dialog/State/DoorDialogState.h"
#include "PaperSprite.h"
#include "Dialog/GuestSubsystem.h"
#include "Engine/GameInstance.h"

void UDialogSpeaker::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	CurrentView = View_1;
	LastView = View_2;

	if (LastView)
	{
		LastView->SetRenderOpacity(0.0f);
		LastView->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (Button)
	{
		Button->OnClicked.AddDynamic(this, &UDialogSpeaker::HandleClicked);
	}
}

void UDialogSpeaker::NativePreConstruct()
{
	if (NamePanel)
	{
		NamePanel->SetVisibility(bShowSpeakerName ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UDialogSpeaker::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button)
	{
		Button->SetIsEnabled(bButtonClickable);
	}

	if (NamePanel)
	{
		NamePanel->SetVisibility(bShowSpeakerName ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UDialogSpeaker::NativeDestruct()
{
	Unbind();
	ReleaseSprites();

	Super::NativeDestruct();
}

void UDialogSpeaker::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bSwapping) { return; }

	SwapProgress = FMath::Min(1.0f, SwapProgress + InDeltaTime * SwapAnimSpeed);

	LastView->SetRenderOpacity(SwapProgress);
	CurrentView->SetRenderOpacity(1.0f - SwapProgress);

	if (SwapProgress >= 1.0f)
	{
		FinishSwap();
	}
}

void UDialogSpeaker::SetGuest(UDoorDialogState* InState, int32 InGuestIndex)
{
	Unbind();

	State = InState;
	GuestIndex = InGuestIndex;
	bLeaving = false;

	if (State)
	{
		State->OnDialogUpdated.AddDynamic(this, &UDialogSpeaker::HandleDialogUpdated);
		State->OnGuestLeaving.AddDynamic(this, &UDialogSpeaker::HandleGuestLeaving);

		FGuestEntry Entry;
		State->GetGuestByIndex(GuestIndex, Entry);

		CurrentGuestID = Entry.GuestData->GetFName();
	}

	if (State && DefaultEmotion.IsValid())
	{
		ApplySprite(State->GetGuestEmotionSprite(GuestIndex, DefaultEmotion.GetTagName()));
	}

	UWorld* World = GetWorld();
	if (World)
	{
		UGuestSubsystem* GuestSubsystem = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UGuestSubsystem>() : nullptr;
		if (GuestSubsystem)
		{
			GuestSubsystem->OnGuestDataChanged.AddDynamic(this, &UDialogSpeaker::HandleGuestDataChanged);
		}
	}

	OnGuestAssigned(GuestIndex);
}

void UDialogSpeaker::ReleaseGuest()
{
	Unbind();

	State = nullptr;
	GuestIndex = INDEX_NONE;
	bLeaving = false;

	CurrentGuestID = FName();

	ReleaseSprites();

	OnRealese();
}

void UDialogSpeaker::NotifyLeaveFinished()
{
	if (!bLeaving) { return; }

	UDoorDialogState* LeavingState = State;
	const int32 LeavingIndex = GuestIndex;

	ReleaseGuest();

	if (LeavingState)
	{
		LeavingState->FinishGuestLeave(LeavingIndex);
	}
}

void UDialogSpeaker::ReleaseSprites()
{
	bSwapping = false;
	SwapProgress = 0.0f;

	CurrentSprite = nullptr;
	PendingSprite = nullptr;

	for (UImage* View : { View_1.Get(), View_2.Get() })
	{
		if (!View) { continue; }

		View->SetBrushResourceObject(nullptr);
		View->SetRenderOpacity(0.0f);
	}

	if (View_2)
	{
		View_2->SetVisibility(ESlateVisibility::Collapsed);
	}

	CurrentView = View_1;
	LastView = View_2;

	if (Button)
	{
		FButtonStyle Style = Button->GetStyle();

		for (FSlateBrush* Target : { &Style.Normal, &Style.Hovered, &Style.Pressed })
		{
			Target->SetResourceObject(nullptr);
		}

		Button->SetStyle(Style);
	}
}

void UDialogSpeaker::Unbind()
{
	if (State)
	{
		State->OnDialogUpdated.RemoveDynamic(this, &UDialogSpeaker::HandleDialogUpdated);
		State->OnGuestLeaving.RemoveDynamic(this, &UDialogSpeaker::HandleGuestLeaving);
	}

	UWorld* World = GetWorld();
	if (!World) { return; }

	UGuestSubsystem* GuestSubsystem = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UGuestSubsystem>() : nullptr;
	if (GuestSubsystem)
	{
		GuestSubsystem->OnGuestDataChanged.RemoveDynamic(this, &UDialogSpeaker::HandleGuestDataChanged);
	}
}

void UDialogSpeaker::HandleGuestDataChanged(FName GuestID)
{
	if (GuestID != CurrentGuestID) { return; }

	FGuestEntry Entry;
	State->GetGuestByIndex(GuestIndex, Entry);

	OnNameUpdated(Entry.GuestData->GuestName, State->GetGuestRealMetrics(Entry.GuestData).bNameKnown);
}

void UDialogSpeaker::HandleClicked()
{
	if (!State || GuestIndex == INDEX_NONE) { return; }

	State->SelectGuest(GuestIndex);
}

void UDialogSpeaker::HandleDialogUpdated(bool bDialogEnded)
{
	if (!bFollowActiveSpeaker || !State || bLeaving) { return; }
	if (State->GetCurrentGuestIndex() != GuestIndex) { return; }

	ApplySprite(State->GetActiveSpeakerSprite());
}

void UDialogSpeaker::HandleGuestLeaving(int32 InGuestIndex)
{
	if (InGuestIndex != GuestIndex || bLeaving) { return; }

	bLeaving = true;
	OnLeave();
}

void UDialogSpeaker::ApplySprite(UPaperSprite* Sprite)
{
	if (!Sprite || !CurrentView || !LastView) { return; }
	if (bSwapping ? Sprite == PendingSprite : Sprite == CurrentSprite) { return; }

	if (bSwapping)
	{
		FinishSwap();
	}

	FSlateBrush Brush;
	Brush.SetResourceObject(Sprite);
	Brush.ImageSize = FVector2D(Sprite->GetSourceSize());

	if (!CurrentSprite)
	{
		CurrentView->SetBrush(Brush);
		CurrentView->SetRenderOpacity(1.0f);
		CurrentView->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		CurrentSprite = Sprite;
		UpdateButtonStyle(Sprite);
		return;
	}

	LastView->SetBrush(Brush);
	LastView->SetRenderOpacity(0.0f);
	LastView->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	PendingSprite = Sprite;
	SwapProgress = 0.0f;
	bSwapping = true;
}

void UDialogSpeaker::FinishSwap()
{
	bSwapping = false;
	SwapProgress = 0.0f;

	Swap(CurrentView, LastView);

	CurrentView->SetRenderOpacity(1.0f);

	LastView->SetRenderOpacity(0.0f);
	LastView->SetVisibility(ESlateVisibility::Collapsed);

	CurrentSprite = PendingSprite;
	UpdateButtonStyle(CurrentSprite);
}

void UDialogSpeaker::UpdateButtonStyle(UPaperSprite* Sprite)
{
	if (!Button || !Sprite) { return; }

	FSlateBrush Brush;
	Brush.SetResourceObject(Sprite);
	Brush.ImageSize = FVector2D(Sprite->GetSourceSize());

	FButtonStyle Style = Button->GetStyle();

	for (FSlateBrush* Target : { &Style.Normal, &Style.Hovered, &Style.Pressed })
	{
		const FSlateColor Tint = Target->TintColor;
		*Target = Brush;
		Target->TintColor = Tint;
	}

	Button->SetStyle(Style);
}
