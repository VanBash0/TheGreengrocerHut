#include "Book/Animation/PageBookmarkAnimationInstance.h"
#include "Book/Actors/PageBookmark.h"

void UPageBookmarkAnimationInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningBookmark = Cast<APageBookmark>(GetOwningActor());
	if (OwningBookmark)
	{
		OwningBookmark->OnHovering.AddDynamic(this, &UPageBookmarkAnimationInstance::HandleHover);
	}
}

void UPageBookmarkAnimationInstance::NativeUpdateAnimation(float DeltaTimeX)
{
	Super::NativeUpdateAnimation(DeltaTimeX);

	CurrentProgress = FMath::FInterpTo(CurrentProgress, TargetProgress, DeltaTimeX, 1.0f);
}

void UPageBookmarkAnimationInstance::HandleHover(bool bHovered)
{
	TargetProgress = bHovered ? 1.0f : 0.0f;
}