#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PageAnimationInstance.generated.h"

class APage;

UCLASS(BlueprintType)
class THEGREENGROCERHUT_API UPageAnimationInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;

protected:
	UFUNCTION()
	void HandleHover(bool bIsHovered);

	UFUNCTION()
	void HandleClick();

	UFUNCTION()
	void AnimNotify_OnFinished();

public:
	UPROPERTY(BlueprintReadOnly, Category = "Page Anim")
	TObjectPtr<APage> OwningPage;
};
