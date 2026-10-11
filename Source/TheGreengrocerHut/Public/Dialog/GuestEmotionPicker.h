#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "Engine/StreamableManager.h"
#include "GuestEmotionPicker.generated.h"

class UGuestData;
class UPaperSprite;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGuestEmotionsLoaded);

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

	UFUNCTION(BlueprintPure, Category = "Guest")
	bool IsLoaded() const { return bLoaded; }

	UPROPERTY(BlueprintAssignable, Category = "Guest")
	FOnGuestEmotionsLoaded OnLoaded;

	virtual void BeginDestroy() override;

private:
	void OnLoadCompleted();

	bool bLoaded = false;

	TMap<FGameplayTag, int32> LastIndexByState;

	UPROPERTY()
	TObjectPtr<const UGuestData> Data;

	TSharedPtr<FStreamableHandle> LoadHandle;
};
