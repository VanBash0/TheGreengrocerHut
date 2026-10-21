#include "Dialog/GuestSubsystem.h"

#include "ClientStruct.h"
#include "Engine/AssetManager.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameLoop.h"
#include "PaperSprite.h"

namespace
{
	const FName GuestDataType = TEXT("GuestData");
	const FName IdleEmotionTag = TEXT("DialogEmotion.Idel");
}

void UGuestSubsystem::EnsureLoaded()
{
	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	if (!AssetManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[UGuestSubsystem] The Asset Manager is not ready, the guests are not loaded."));
		return;
	}

	TArray<FPrimaryAssetId> Ids;
	AssetManager->GetPrimaryAssetIdList(FPrimaryAssetType(GuestDataType), Ids);

	GuestIds.Reset();
	for (const FPrimaryAssetId& Id : Ids)
	{
		GuestIds.Add(Id.PrimaryAssetName);
	}
	GuestIds.Sort([](const FName& A, const FName& B) { return A.LexicalLess(B); });

	for (const FPrimaryAssetId& Id : Ids)
	{
		if (GuestsById.Contains(Id.PrimaryAssetName)) { continue; }

		UGuestData* Guest = Cast<UGuestData>(AssetManager->GetPrimaryAssetPath(Id).TryLoad());
		if (!Guest)
		{
			UE_LOG(LogTemp, Warning, TEXT("[UGuestSubsystem] Guest '%s' failed to load, skipped."), *Id.ToString());
			continue;
		}

		Guests.Add(Guest);
		GuestsById.Add(Guest->GetFName(), Guest);
	}

	UE_LOG(LogTemp, Log, TEXT("[UGuestSubsystem] %d of %d guest(s) in memory."), Guests.Num(), GuestIds.Num());
}

const UGuestData* UGuestSubsystem::FindGuest(FName Id) const
{
	const TWeakObjectPtr<UGuestData>* Found = GuestsById.Find(Id);
	return Found ? Found->Get() : nullptr;
}

void UGuestSubsystem::Trim(const TArray<FClient>& DayClients, const USaveGameData* Save)
{
	EnsureLoaded();

	if (!Save) { return; }

	TSet<FName> Today;
	for (const FClient& Client : DayClients)
	{
		for (const TObjectPtr<UGuestData>& Guest : Client.Guests)
		{
			if (Guest) { Today.Add(Guest->GetFName()); }
		}
	}

	TArray<const UGuestData*> Needed;

	for (int32 i = Guests.Num() - 1; i >= 0; --i)
	{
		const UGuestData* Guest = Guests[i];
		if (!Guest) { Guests.RemoveAtSwap(i); continue; }

		const FGuestMetrics* Metrics = Save->GuestMetrics.Find(Guest->GetFName());
		const bool bSeen = Metrics && Metrics->bSeen;

		if (bSeen || Today.Contains(Guest->GetFName()))
		{
			Needed.Add(Guest);
		}
		else
		{
			GuestsById.Remove(Guest->GetFName());
			Guests.RemoveAtSwap(i);
		}
	}

	// The avatars of those who stay: the visited ones for the book, the ones of today for the moment they come
	LoadIdleSprites(Needed);

	UE_LOG(LogTemp, Log, TEXT("[UGuestSubsystem] Trim: %d guest(s) stay in memory."), Guests.Num());
}

TSharedPtr<FStreamableHandle> UGuestSubsystem::LoadDialogue(const UGuestData* Guest, UObject* Owner, TFunction<void(UDlgDialogue*)> OnLoaded)
{
	if (!Guest || Guest->GuestDialogue.IsNull() || !Owner)
	{
		if (OnLoaded) { OnLoaded(nullptr); }
		return nullptr;
	}

	const TSoftObjectPtr<UDlgDialogue> Dialogue = Guest->GuestDialogue;

	return UAssetManager::GetStreamableManager().RequestAsyncLoad(Dialogue.ToSoftObjectPath(),
		FStreamableDelegate::CreateWeakLambda(Owner, [Dialogue, OnLoaded]()
		{
			if (OnLoaded) { OnLoaded(Dialogue.Get()); }
		}));
}

TSharedPtr<FStreamableHandle> UGuestSubsystem::LoadEmotions(const UGuestData* Guest, UObject* Owner, TFunction<void()> OnLoaded)
{
	TArray<FSoftObjectPath> Paths;

	if (Guest)
	{
		for (const TPair<FGameplayTag, TInstancedStruct<FGuestSpriteBase>>& Pair : Guest->Emotions)
		{
			if (const FGuestSingleSprite* Single = Pair.Value.GetPtr<FGuestSingleSprite>())
			{
				if (!Single->Sprite.IsNull()) { Paths.AddUnique(Single->Sprite.ToSoftObjectPath()); }
			}
			else if (const FGuestVariationSprite* Variation = Pair.Value.GetPtr<FGuestVariationSprite>())
			{
				for (const TSoftObjectPtr<UPaperSprite>& Sprite : Variation->Sprites)
				{
					if (!Sprite.IsNull()) { Paths.AddUnique(Sprite.ToSoftObjectPath()); }
				}
			}
		}
	}

	if (Paths.IsEmpty() || !Owner)
	{
		if (OnLoaded) { OnLoaded(); }
		return nullptr;
	}

	return UAssetManager::GetStreamableManager().RequestAsyncLoad(Paths,
		FStreamableDelegate::CreateWeakLambda(Owner, [OnLoaded]()
		{
			if (OnLoaded) { OnLoaded(); }
		}));
}

TSoftObjectPtr<UPaperSprite> UGuestSubsystem::FindIdleSprite(const UGuestData* Guest)
{
	const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(IdleEmotionTag, false);
	if (!Guest || !Tag.IsValid()) { return nullptr; }

	const TInstancedStruct<FGuestSpriteBase>* Emotion = Guest->Emotions.Find(Tag);
	if (!Emotion) { return nullptr; }

	if (const FGuestSingleSprite* Single = Emotion->GetPtr<FGuestSingleSprite>())
	{
		return Single->Sprite;
	}

	if (const FGuestVariationSprite* Variation = Emotion->GetPtr<FGuestVariationSprite>())
	{
		if (!Variation->Sprites.IsEmpty()) { return Variation->Sprites[0]; }
	}

	return nullptr;
}

void UGuestSubsystem::LoadIdleSprites(const TArray<const UGuestData*>& ForGuests)
{
	SpriteHandles.RemoveAll([](const TSharedPtr<FStreamableHandle>& Handle) { return !Handle.IsValid() || Handle->HasLoadCompleted(); });

	TMap<FName, FSoftObjectPath> Missing;
	for (const UGuestData* Guest : ForGuests)
	{
		if (!Guest || IdleSprites.Contains(Guest->GetFName())) { continue; }

		const FSoftObjectPath Path = FindIdleSprite(Guest).ToSoftObjectPath();
		if (!Path.IsNull())
		{
			Missing.Add(Guest->GetFName(), Path);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[UGuestSubsystem] Guest '%s' has no idle sprite: no '%s' in Emotions (or the tag does not exist)."),
				*Guest->GetName(), *IdleEmotionTag.ToString());
		}
	}

	if (Missing.IsEmpty()) { return; }

	TArray<FSoftObjectPath> Paths;
	Missing.GenerateValueArray(Paths);

	TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(Paths,
		FStreamableDelegate::CreateWeakLambda(this, [this, Missing]()
		{
			for (const TPair<FName, FSoftObjectPath>& Pair : Missing)
			{
				if (UPaperSprite* Sprite = Cast<UPaperSprite>(Pair.Value.ResolveObject()))
				{
					IdleSprites.Add(Pair.Key, Sprite);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[UGuestSubsystem] Idle sprite '%s' of guest '%s' did not load."), *Pair.Value.ToString(), *Pair.Key.ToString());
				}
			}
		}));

	if (Handle.IsValid() && !Handle->HasLoadCompleted())
	{
		SpriteHandles.Add(Handle);
	}
}

const USaveGameData* UGuestSubsystem::GetSave() const
{
	const UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	const UGameLoop* Loop = World ? World->GetSubsystem<UGameLoop>() : nullptr;
	return Loop ? Loop->GetSavedData() : nullptr;
}

TArray<FName> UGuestSubsystem::GetSeenIds(const USaveGameData* Save) const
{
	TArray<FName> Seen;
	if (!Save) { return Seen; }

	for (const FName& Id : GuestIds)
	{
		const FGuestMetrics* Metrics = Save->GuestMetrics.Find(Id);
		if (Metrics && Metrics->bSeen) { Seen.Add(Id); }
	}

	return Seen;
}

FName UGuestSubsystem::GetGuestIdByIndex(int32 Index) const
{
	const TArray<FName> Seen = GetSeenIds(GetSave());
	return Seen.IsValidIndex(Index) ? Seen[Index] : NAME_None;
}

bool UGuestSubsystem::GetGuestByIndex(int32 Index, FGuestInfo& OutInfo)
{
	OutInfo = FGuestInfo();

	const USaveGameData* Save = GetSave();
	if (!Save) { return false; }

	const TArray<FName> Met = GetSeenIds(Save);
	if (!Met.IsValidIndex(Index)) { return false; }

	FGuestInfo Info;
	Info.Id = Met[Index];

	const FGuestMetrics* Saved = Save->GuestMetrics.Find(Info.Id);

	const UGuestData* Guest = FindGuest(Info.Id);
	if (!Guest)
	{
		UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
		UGuestData* Loaded = AssetManager ? Cast<UGuestData>(AssetManager->GetPrimaryAssetPath(FPrimaryAssetId(GuestDataType, Info.Id)).TryLoad()) : nullptr;
		if (!Loaded) { return false; }

		Guests.Add(Loaded);
		GuestsById.Add(Loaded->GetFName(), Loaded);
		Guest = Loaded;
	}

	// The name is given only if the player knows it: the page shows "???" for an empty one
	Info.Name = Saved->bNameKnown ? Guest->GuestName : FText::GetEmpty();
	Info.Metrics = *Saved;
	Info.DescriptionParts = Guest->DescriptionParts;
	Info.NotesParts = Guest->NotesParts;

	if (const TObjectPtr<UPaperSprite>* Sprite = IdleSprites.Find(Info.Id))
	{
		Info.IdleSprite = *Sprite;
	}
	else
	{
		if (UPaperSprite* Loaded = FindIdleSprite(Guest).LoadSynchronous())
		{
			IdleSprites.Add(Info.Id, Loaded);
			Info.IdleSprite = Loaded;
		}
	}

	OutInfo = Info;
	return true;
}
