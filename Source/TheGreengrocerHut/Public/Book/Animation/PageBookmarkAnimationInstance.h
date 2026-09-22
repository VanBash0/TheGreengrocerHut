#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PageBookmarkAnimationInstance.generated.h"

class APageBookmark;

UCLASS(BlueprintType)
class THEGREENGROCERHUT_API UPageBookmarkAnimationInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaTimeX) override;

protected:
	UFUNCTION(BlueprintCallable, Category = "Bookmark Anim")
	void HandleHover(bool bHovered);

public:
	UPROPERTY(BlueprintReadOnly, Category = "Bookmark Anim")
	TObjectPtr<APageBookmark> OwningBookmark;

	UPROPERTY(BlueprintReadWrite, Category = "Bookmark Anim")
	float CurrentProgress;

	UPROPERTY(BlueprintReadWrite, Category = "Bookmark Anim")
	float TargetProgress;
};
