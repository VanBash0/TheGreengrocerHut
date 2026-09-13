#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PageAnimationInstance.generated.h"

UCLASS()
class THEGREENGROCERHUT_API UPageAnimationInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;

	UPROPERTY(BlueprintReadOnly, Category = "Page Anim")
	TObjectPtr<APage> OwningPage;

protected:
	UFUNCTION()
	void HandleHover(bool bIsHovered);

	UFUNCTION()
	void HandleClick();

	UFUNCTION()
	void AnimNotify_OnFinished();
};
