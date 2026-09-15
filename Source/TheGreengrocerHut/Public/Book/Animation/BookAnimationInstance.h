#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BookAnimationInstance.generated.h"

class ABook;

UCLASS(BlueprintType)
class THEGREENGROCERHUT_API UBookAnimationInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaTimeX) override;

	UPROPERTY(BlueprintReadOnly, Category = "Book Anim")
	TObjectPtr<ABook> OwningBook;

protected:
	UFUNCTION()
	void OnPageChanged(int32 PrevPage, int32 CurPage);

	UFUNCTION()
	void AnimNotify_OnStart();

	UFUNCTION()
	void AnimNotify_OnEnd();

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Book Anim")
	int32 KeyCount;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Book Anim")
	float TargetFadeTime;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Book Anim")
	float CurrentFadeTime;
};
