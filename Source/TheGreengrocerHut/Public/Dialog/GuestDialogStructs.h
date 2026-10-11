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

USTRUCT(BlueprintType)
struct FGuestMetrics
{ 
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool GuestAlive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGuestFraction GuestFraction;
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UDlgDialogue> GuestDialogue;
};
