#include "Dialog/GuestEmotionPicker.h"
#include "Dialog/GuestDialogStructs.h"

void UGuestEmotionPicker::Initialize(const UGuestData* InData)
{
	LastIndexByState.Reset();
	Data = InData;
}

void UGuestEmotionPicker::Release()
{
	LastIndexByState.Reset();
	Data = nullptr;
}

UPaperSprite* UGuestEmotionPicker::GetEmotionSprite(const FName& StateName)
{
	if (!Data) { return nullptr; }

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
