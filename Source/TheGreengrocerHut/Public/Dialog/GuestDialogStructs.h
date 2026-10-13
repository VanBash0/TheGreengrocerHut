#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PaperSprite.h"
#include "DlgDialogue.h"
#include "StructUtils/InstancedStruct.h"
#include "GuestDialogStructs.generated.h"

UENUM(BlueprintType)
enum class EGuestFraction : uint8 
{
	Peasant    UMETA(DisplayName = "Крестьянин"),
	Boyar      UMETA(DisplayName = "Боярин"),
	Righteous  UMETA(DisplayName = "Праведник"),
	Sorcerer   UMETA(DisplayName = "Ведуны")
};

inline EGuestFraction GetOppositeFraction(EGuestFraction In)
{
	switch (In)
	{
		case EGuestFraction::Peasant:   return EGuestFraction::Boyar;
		case EGuestFraction::Boyar:     return EGuestFraction::Peasant;
		case EGuestFraction::Righteous: return EGuestFraction::Sorcerer;
		case EGuestFraction::Sorcerer:  return EGuestFraction::Righteous;
	}

	return In;
}

UENUM(BlueprintType)
enum class EGuestOutcome : uint8
{
	Healed    UMETA(DisplayName = "Вылечил"),
	NotHealed UMETA(DisplayName = "Недолечил"),
	Poisoned  UMETA(DisplayName = "Отравил"),
	Rejected  UMETA(DisplayName = "Непринял")
};

USTRUCT(BlueprintType)
struct FGuestMetrics
{ 
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGuestFraction GuestFraction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool GuestAlive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EGuestOutcome> Outcomes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 VisitCount = 0;
};

USTRUCT(BlueprintType)
struct FGuestSpriteBase
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FGuestSingleSprite : public FGuestSpriteBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UPaperSprite> Sprite;
};

USTRUCT(BlueprintType)
struct FGuestVariationSprite : public FGuestSpriteBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSoftObjectPtr<UPaperSprite>> Sprites;
};

USTRUCT(BlueprintType)
struct FReputationDelta
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "Own", ToolTip = "Изменение репутации у самой фракции гостя"))
	int32 Own = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "Opposing", ToolTip = "Изменение репутации у противоположной фракции (Крестьяне и Бояре, Праведники и Ведуны)"))
	int32 Opposing = 0;
};

USTRUCT(BlueprintType)
struct FFractionReputationRules
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "By Outcome",
			ToolTip = "Как меняется репутация в зависимости от того, как игрок поступил с гостем этой фракции. Если исхода нет в таблице, репутация не меняется"))
	TMap<EGuestOutcome, FReputationDelta> ByOutcome;
};

USTRUCT(BlueprintType)
struct FGuestReputationModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "Для какого исхода применяется множитель"))
	EGuestOutcome TriggerOutcome = EGuestOutcome::Healed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "Множитель дельты репутации (1.15 = на 15% больше, 0.75 = штраф на 25% мягче)"))
	float Multiplier = 1.0f;
};

UCLASS(BlueprintType)
class UGuestData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText GuestName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ETextGender GuestGeneder;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGuestMetrics DefaultMetrics;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "DialogEmotion", ExcludeBaseStruct))
	TMap<FGameplayTag, TInstancedStruct<FGuestSpriteBase>> Emotions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UDlgDialogue> GuestDialogue;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reputation", meta = (InlineEditConditionToggle))
	bool bOverrideReputation = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reputation", meta = (EditCondition = "bOverrideReputation"))
	FFractionReputationRules OverrideReputationRules;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guest Outcome",
		meta = (ClampMin = "0", ToolTip = "Сколько визитов гость гарантированно выживает. Шанс смерти начинает действовать со следующего визита (при 2: визиты 1 и 2 безопасны). 0 = шанс смерти действует всегда"))
	int32 MinVisitsBeforeDeath = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guest Outcome", meta = (InlineEditConditionToggle))
	bool bOverrideDeathChance = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guest Outcome", meta = (EditCondition = "bOverrideDeathChance"))
	TMap<EGuestOutcome, float> OverrideDeathChance;
};
