#include "Dialog/GuestEmotionPicker.h"
#include "Dialog/GuestDialogStructs.h"
#include "Engine/AssetManager.h"

void UGuestEmotionPicker::Initialize(const UGuestData* InData)
{
	Release();
	LastIndexByState.Reset();
	Data = InData;

	if (!Data) { return; }

	TArray<FSoftObjectPath> Paths;
	for (const TPair<FGameplayTag, TInstancedStruct<FGuestSpriteBase>>& Pair : Data->Emotions)
	{
		if (const FGuestSingleSprite* Single = Pair.Value.GetPtr<FGuestSingleSprite>())
		{
			if (!Single->Sprite.IsNull())
			{
				Paths.AddUnique(Single->Sprite.ToSoftObjectPath());
			}
		}
		else if (const FGuestVariationSprite* Variation = Pair.Value.GetPtr<FGuestVariationSprite>())
		{
			for (const TSoftObjectPtr<UPaperSprite>& Sprite : Variation->Sprites)
			{
				if (!Sprite.IsNull())
				{
					Paths.AddUnique(Sprite.ToSoftObjectPath());
				}
			}
		}
	}

	if (Paths.Num() > 0)
	{
		LoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(Paths, FStreamableDelegate::CreateUObject(this, &UGuestEmotionPicker::OnLoadCompleted));
	}
	else
	{
		OnLoadCompleted();
	}
}

void UGuestEmotionPicker::Release()
{
	if (LoadHandle.IsValid())
	{
		if (LoadHandle->IsLoadingInProgress())
		{
			LoadHandle->CancelHandle();
		}
		else
		{
			LoadHandle->ReleaseHandle();
		}
		LoadHandle.Reset();
	}

	bLoaded = false;
}

void UGuestEmotionPicker::BeginDestroy()
{
	Release();
	Super::BeginDestroy();
}

void UGuestEmotionPicker::OnLoadCompleted()
{
	bLoaded = true;
	OnLoaded.Broadcast();
}

UPaperSprite* UGuestEmotionPicker::GetEmotionSprite(const FName& StateName)
{
	if (!Data || !bLoaded) { return nullptr; }

	const FGameplayTag State = FGameplayTag::RequestGameplayTag(StateName, false);
	if (!State.IsValid()) { return nullptr; }

	const TInstancedStruct<FGuestSpriteBase>* Entry = Data->Emotions.Find(State);
	if (!Entry) { return nullptr; }

	if (const FGuestSingleSprite* Single = Entry->GetPtr<FGuestSingleSprite>()) { return Single->Sprite.Get(); }

	if (const FGuestVariationSprite* Variation = Entry->GetPtr<FGuestVariationSprite>())
	{
		const int32 Num = Variation->Sprites.Num();
		if (Num == 0)
		{
			return nullptr;
		}
		if (Num == 1)
		{
			LastIndexByState.Add(State, 0);
			return Variation->Sprites[0].Get();
		}

		int32 Index = FMath::RandRange(0, Num - 1);
		if (const int32* Last = LastIndexByState.Find(State))
		{
			Index = FMath::RandRange(0, Num - 2);
			if (Index >= *Last)
			{
				++Index;
			}
		}

		LastIndexByState.Add(State, Index);
		return Variation->Sprites[Index].Get();
	}

	return nullptr;
}
