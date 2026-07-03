#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "IngredientStructures.h"
#include "GardenStructures.generated.h"

USTRUCT(BlueprintType)
struct FGrowPhase : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Grow Day Number", MakeStructureDefaultValue = "None", ClampMax = "5", UIMin = "0", UIMax = "5"))
	int32 GrowDayNumber;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Mesh", MakeStructureDefaultValue = "None"))
	TObjectPtr<UStaticMesh> Mesh;
};

UENUM(BlueprintType)
enum class EPlantHarvestType : uint8
{
	SingleHarvest,
	MultipleHarvest
};

USTRUCT(BlueprintType)
struct FSeedDescription
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Description")
	FText Name;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Description", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Mesh", MakeStructureDefaultValue = "None"))
	TObjectPtr<UStaticMesh> Mesh;	

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Growth")
	TObjectPtr<UIngredientContainer> SeedContainer;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Description")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Description")
	TObjectPtr<USoundCue> SFX;
};

USTRUCT(BlueprintType)
struct FIngredientSeed : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Description")
	FSeedDescription Info;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Growth")
	EPlantHarvestType HarvestType = EPlantHarvestType::SingleHarvest;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Growth")
	TArray<FGrowPhase> GrowthPhases;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Harvest")
	FInt32Interval HarvestAmountRange = FInt32Interval(1, 1);

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Harvest")
	FIngredientRowNameRef GrowedIngredientRowName;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Regrow", meta = (EditCondition = "HarvestType == EPlantHarvestType::MultipleHarvest"))
	int32 DayAmountBetweenHarvest = 0;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Regrow", meta = (EditCondition = "HarvestType == EPlantHarvestType::MultipleHarvest"))
	int32 MaxHarvestAmount = 0;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Regrow", meta = (EditCondition = "HarvestType == EPlantHarvestType::MultipleHarvest"))
	int32 RegrowVisualPhaseIndex = 0;
};