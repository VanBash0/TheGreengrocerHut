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
	Sorcerer   UMETA(DisplayName = "Ведуны"),
	Demon      UMETA(DisplayName = "Демон")
};

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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "Reaction", ToolTip = "Реакция фракций на событие", InvalidEnumValues = "Demon"))
	TMap<EGuestFraction, int32> Reaction;
};

USTRUCT(BlueprintType)
struct FFractionReputationRules
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "By Outcome", ToolTip = "Как меняется репутация в зависимости от того, как игрок поступил с гостем этой фракции. Если исхода нет в таблице, репутация не меняется"))
	TMap<EGuestOutcome, FReputationDelta> ByOutcome;
};

USTRUCT(BlueprintType)
struct FReputationChange
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	EGuestFraction Fraction = EGuestFraction::Peasant;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "На сколько изменилась репутация"))
	int32 Delta = 0;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "Значение репутации после изменения"))
	int32 NewValue = 0;
};

USTRUCT(BlueprintType)
struct FGuestOutcomeResult
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "Жив ли гость после этого поступка"))
	bool bGuestAlive = true;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "Что изменилось в репутации (пусто, если правила для этого исхода нет)"))
	TArray<FReputationChange> ReputationChanges;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "Множитель, применённый ко всем изменениям репутации от этого поступка"))
	float AppliedMultiplier = 1.0f;
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
