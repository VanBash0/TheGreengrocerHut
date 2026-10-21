#include "Dialog/GuestBookSubsystem.h"

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

void UGuestBookSubsystem::Refresh()
{
	++RequestId;
	bReady = false;

	Entries.Reset();
	SpritePaths.Reset();
	GuestsHandle.Reset();
	SpritesHandle.Reset();

	const TArray<FSoftObjectPath> Paths = CollectMetGuestPaths();
	if (Paths.IsEmpty())
	{
		Finish();
		return;
	}

	const int32 Id = RequestId;
	GuestsHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(Paths,
		FStreamableDelegate::CreateWeakLambda(this, [this, Id]() { OnGuestsLoaded(Id); }));
}

int32 UGuestBookSubsystem::GetTotalGuests() const
{
	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	if (!AssetManager) { return 0; }

	TArray<FPrimaryAssetId> Ids;
	AssetManager->GetPrimaryAssetIdList(FPrimaryAssetType(GuestDataType), Ids);

	return Ids.Num();
}

TArray<FSoftObjectPath> UGuestBookSubsystem::CollectMetGuestPaths() const
{
	TArray<FSoftObjectPath> Paths;

	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	const UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	const UGameLoop* Loop = World ? World->GetSubsystem<UGameLoop>() : nullptr;
	const USaveGameData* Save = Loop ? Loop->GetSavedData() : nullptr;

	if (!AssetManager || !Save) { return Paths; }

	TArray<FPrimaryAssetId> Ids;
	AssetManager->GetPrimaryAssetIdList(FPrimaryAssetType(GuestDataType), Ids);

	for (const FPrimaryAssetId& Id : Ids)
	{
		const FGuestMetrics* Metrics = Save->GuestMetrics.Find(Id.PrimaryAssetName);
		if (Metrics && Metrics->VisitCount > 0)
		{
			Paths.Add(AssetManager->GetPrimaryAssetPath(Id));
		}
	}

	return Paths;
}

TSoftObjectPtr<UPaperSprite> UGuestBookSubsystem::FindIdleSprite(const UGuestData* Guest)
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

void UGuestBookSubsystem::OnGuestsLoaded(int32 InRequestId)
{
	if (InRequestId != RequestId || !GuestsHandle.IsValid()) { return; }

	const UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	const UGameLoop* Loop = World ? World->GetSubsystem<UGameLoop>() : nullptr;

	TArray<UObject*> Loaded;
	GuestsHandle->GetLoadedAssets(Loaded);

	for (UObject* Object : Loaded)
	{
		const UGuestData* Guest = Cast<UGuestData>(Object);
		if (!Guest) { continue; }

		const FGuestMetrics Metrics = Loop ? Loop->GetGuestMetrics(Guest) : Guest->DefaultMetrics;

		FGuestBookEntry& Entry = Entries.AddDefaulted_GetRef();
		Entry.Id = Guest->GetFName();
		Entry.Name = Guest->GuestName;
		Entry.Fraction = Guest->DefaultMetrics.GuestFraction;
		Entry.VisitCount = Metrics.VisitCount;
		Entry.bAlive = Metrics.GuestAlive;

		SpritePaths.Add(FindIdleSprite(Guest).ToSoftObjectPath());
	}

	TArray<FSoftObjectPath> ToLoad;
	for (const FSoftObjectPath& Path : SpritePaths)
	{
		if (!Path.IsNull()) { ToLoad.AddUnique(Path); }
	}

	if (ToLoad.IsEmpty())
	{
		Finish();
		return;
	}

	const int32 Id = RequestId;
	SpritesHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(ToLoad,
		FStreamableDelegate::CreateWeakLambda(this, [this, Id]() { OnSpritesLoaded(Id); }));
}

void UGuestBookSubsystem::OnSpritesLoaded(int32 InRequestId)
{
	if (InRequestId != RequestId) { return; }

	for (int32 i = 0; i < Entries.Num() && i < SpritePaths.Num(); ++i)
	{
		Entries[i].IdleSprite = Cast<UPaperSprite>(SpritePaths[i].ResolveObject());
	}

	Finish();
}

void UGuestBookSubsystem::Finish()
{
	Entries.Sort([](const FGuestBookEntry& A, const FGuestBookEntry& B) { return A.Id.LexicalLess(B.Id); });

	bReady = true;
	OnReady.Broadcast();
}
