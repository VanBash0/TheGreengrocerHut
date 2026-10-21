#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/StreamableManager.h"
#include "Dialog/GuestDialogStructs.h"
#include "GuestSubsystem.generated.h"

class UPaperSprite;
class USaveGameData;
struct FClient;

USTRUCT(BlueprintType)
struct FGuestInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FName Id;

	UPROPERTY(BlueprintReadOnly)
	FText Name;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPaperSprite> IdleSprite;

	UPROPERTY(BlueprintReadOnly)
	FGuestMetrics Metrics;

	UPROPERTY(BlueprintReadOnly)
	TArray<FText> DescriptionParts;

	UPROPERTY(BlueprintReadOnly)
	TArray<FText> NotesParts;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGuestDataChanged, FName, GuestId);

UCLASS()
class THEGREENGROCERHUT_API UGuestSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void EnsureLoaded();

	void Trim(const TArray<FClient>& DayClients, const USaveGameData* Save);

	TSharedPtr<FStreamableHandle> LoadDialogue(const UGuestData* Guest, UObject* Owner, TFunction<void(UDlgDialogue*)> OnLoaded);

	TSharedPtr<FStreamableHandle> LoadEmotions(const UGuestData* Guest, UObject* Owner, TFunction<void()> OnLoaded);

	const TArray<TObjectPtr<UGuestData>>& GetAllGuests() const { return Guests; }

	const UGuestData* FindGuest(FName Id) const;

public:
	UFUNCTION(BlueprintPure, Category = "Guest")
	int32 GetTotalGuests() const { return GuestIds.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Guest")
	bool GetGuestByIndex(int32 Index, FGuestInfo& OutInfo);

	// The name of the guest of the page (None if there is no such guest). Cheap: nothing is loaded.
	// The page keeps the name of the guest it shows and compares it in OnGuestDataChanged
	UFUNCTION(BlueprintPure, Category = "Guest")
	FName GetGuestIdByIndex(int32 Index) const;

	// Something about the guest GuestId has changed (seen, name, description, note, outcome ...).
	// The pages of the book listen to it: refresh if GuestId is your guest or if the guest of your index is not the one you show
	UPROPERTY(BlueprintAssignable, Category = "Guest")
	FOnGuestDataChanged OnGuestDataChanged;

	void NotifyDataChanged(FName GuestId) { OnGuestDataChanged.Broadcast(GuestId); }

private:
	// The guests the player has seen, in the order of the pages
	TArray<FName> GetSeenIds(const USaveGameData* Save) const;

	const USaveGameData* GetSave() const;

	void LoadIdleSprites(const TArray<const UGuestData*>& ForGuests);

	static TSoftObjectPtr<UPaperSprite> FindIdleSprite(const UGuestData* Guest);

private:
	UPROPERTY()
	TArray<TObjectPtr<UGuestData>> Guests;

	TMap<FName, TWeakObjectPtr<UGuestData>> GuestsById;

	TArray<FName> GuestIds;

	UPROPERTY()
	TMap<FName, TObjectPtr<UPaperSprite>> IdleSprites;

	TArray<TSharedPtr<FStreamableHandle>> SpriteHandles;
};
