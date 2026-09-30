#include "Book/Animation/BookAnimationInstance.h"
#include "Book/Actors/Book.h"
#include "Book/Data/BookData.h"

void UBookAnimationInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningBook = Cast<ABook>(GetOwningActor());
	if (OwningBook)
	{
		OwningBook->OnPageChanged.AddDynamic(this, &UBookAnimationInstance::OnPageChanged);
	}
}

void UBookAnimationInstance::NativeUpdateAnimation(float DeltaTimeX)
{
	Super::NativeUpdateAnimation(DeltaTimeX);

	const float InterpSpeed = OwningBook->bIsFlippingSequenceActive ? FakePageInterpolateSpeedFast : FakePageInterpolateSpeedDefault;
	CurrentFadeTime = FMath::FInterpTo(CurrentFadeTime, TargetFadeTime, DeltaTimeX, InterpSpeed);
}

void UBookAnimationInstance::OnPageChanged(int32 PrevPage, int32 CurPage)
{
	if (!OwningBook || !OwningBook->BookData || !OwningBook->BookData->Cover) { return; }

	UCoverData* data = OwningBook->BookData->Cover;

	if (PrevPage == -1)
	{
		Montage_Play(data->AM_Open_Front, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		KeyCount = 0;
	}
	else if (CurPage == -1)
	{
		Montage_Play(data->AM_Open_Front, -1.0f, EMontagePlayReturnType::MontageLength, 1.0f, true);
		KeyCount = 0;
	}
	else if (CurPage > OwningBook->TotalPageCount)
	{
		Montage_Play(data->AM_Close_Back, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		KeyCount = 0;
	}
	else if (PrevPage > OwningBook->TotalPageCount)
	{
		Montage_Play(data->AM_Close_Back, -1.0f, EMontagePlayReturnType::MontageLength, 1.0f, true);
		KeyCount = 0;
	}
	else
	{
		const float Denominator = FMath::Max(1.0f, static_cast<float>((OwningBook->TotalPageCount - 2) - 2));
		TargetFadeTime = FMath::Clamp((OwningBook->CurPageNumber - 2) / Denominator, 0.0f, 1.0f);
		Montage_Stop(0.0f);
	}
}

void UBookAnimationInstance::AnimNotify_OnStart()
{
	KeyCount++;
	OwningBook->bCanFlipPage = KeyCount >= 2;
}

void UBookAnimationInstance::AnimNotify_OnEnd()
{
	KeyCount++;
	OwningBook->bCanFlipPage = KeyCount >= 2;
}