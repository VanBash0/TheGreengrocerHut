#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "GuestEmotionPicker.generated.h"

class UGuestData;
class UPaperSprite;

// Picks the sprite of the guest for an emotion (no repeat of the same variation twice in a row).
// Does not load anything: the sprites are loaded by UGuestSubsystem::LoadEmotions, an unloaded sprite gives nullptr
UCLASS(BlueprintType)
class THEGREENGROCERHUT_API UGuestEmotionPicker : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Guest")
	void Initialize(const UGuestData* InData);

	UFUNCTION(BlueprintCallable, Category = "Guest")
	void Release();

	UFUNCTION(BlueprintCallable, Category = "Guest")
	UPaperSprite* GetEmotionSprite(const FName& StateName);

private:
	TMap<FGameplayTag, int32> LastIndexByState;

	UPROPERTY()
	TObjectPtr<const UGuestData> Data;
};
