#include "Book/Animation/PageAnimationInstance.h"
#include "Book/Actors/Page.h"
#include "Book/Data/PageData.h"

void UPageAnimationInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningPage = Cast<APage>(GetOwningActor());
	if (OwningPage)
	{
		OwningPage->OnHovering.AddDynamic(this, &UPageAnimationInstance::HandleHover);
		OwningPage->OnClick.AddDynamic(this, &UPageAnimationInstance::HandleClick);
	}
}

void UPageAnimationInstance::HandleHover(bool bIsHovered)
{
	if (!OwningPage || !OwningPage->PageData) { return; }

	UAnimMontage* MontageToPlay = OwningPage->bOnRightSide ? OwningPage->PageData->Page_Take_R : OwningPage->PageData->Page_Take_L;

	if (!MontageToPlay) { return; }

	const float InPlayRate = bIsHovered ? 1.0f : -1.0f;
	const float InTimeToStartMontageAt = bIsHovered ? 0.0f : 1.0f;

	Montage_Play(MontageToPlay, InPlayRate, EMontagePlayReturnType::MontageLength, InTimeToStartMontageAt, true);
}

void UPageAnimationInstance::HandleClick()
{
	if (!OwningPage || !OwningPage->PageData) { return; }

	UAnimMontage* MontageToPlay = OwningPage->bOnRightSide ? OwningPage->PageData->Page_Flip_R : OwningPage->PageData->Page_Flip_L;

	if (MontageToPlay)
	{
		Montage_Play(MontageToPlay);
	}
}

void UPageAnimationInstance::AnimNotify_OnFinished()
{
	if (OwningPage)
	{
		OwningPage->OnFinish();
	}
}

