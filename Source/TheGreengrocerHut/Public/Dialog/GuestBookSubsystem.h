#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/StreamableManager.h"
#include "Dialog/GuestDialogStructs.h"
#include "GuestBookSubsystem.generated.h"

class UPaperSprite;

// Ready data of one guest for the book page. BP never touches UGuestData
USTRUCT(BlueprintType)
struct FGuestBookEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FName Id;

	UPROPERTY(BlueprintReadOnly)
	FText Name;

	// The fraction the guest shows (not the real one)
	UPROPERTY(BlueprintReadOnly)
	EGuestFraction Fraction = EGuestFraction::Peasant;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPaperSprite> IdleSprite;

	UPROPERTY(BlueprintReadOnly)
	int32 VisitCount = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bAlive = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGuestBookReady);

// Loads (async) the guests who already visited and their idle sprites. Call Refresh, wait for OnReady, then read the list
UCLASS()
class THEGREENGROCERHUT_API UGuestBookSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "GuestBook")
	void Refresh();

	// Every guest that can come (real demons included). No loading needed
	UFUNCTION(BlueprintPure, Category = "GuestBook")
	int32 GetTotalGuests() const;

	UFUNCTION(BlueprintPure, Category = "GuestBook")
	bool IsReady() const { return bReady; }

	UFUNCTION(BlueprintPure, Category = "GuestBook")
	int32 GetMetCount() const { return Entries.Num(); }

	// Guests who already visited. Valid after OnReady
	UFUNCTION(BlueprintPure, Category = "GuestBook")
	const TArray<FGuestBookEntry>& GetMetGuests() const { return Entries; }

public:
	UPROPERTY(BlueprintAssignable, Category = "GuestBook")
	FOnGuestBookReady OnReady;

private:
	TArray<FSoftObjectPath> CollectMetGuestPaths() const;

	void OnGuestsLoaded(int32 InRequestId);
	void OnSpritesLoaded(int32 InRequestId);
	void Finish();

	static TSoftObjectPtr<UPaperSprite> FindIdleSprite(const UGuestData* Guest);

private:
	UPROPERTY()
	TArray<FGuestBookEntry> Entries;

	TArray<FSoftObjectPath> SpritePaths;

	TSharedPtr<FStreamableHandle> GuestsHandle;
	TSharedPtr<FStreamableHandle> SpritesHandle;

	// Drops the answers of the old Refresh
	int32 RequestId = 0;

	bool bReady = false;
};
