#pragma once

#include "CoreMinimal.h"
#include "SymptomStructures.h"
#include "ClientStruct.h"
#include "NewsPaperStructures.generated.h"

USTRUCT(BlueprintType)
struct FNewspaper
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "VillageImage", MakeStructureDefaultValue = "None"))
	TObjectPtr<UTexture2D> VillageImage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Description"))
	FText Description;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Demon Symptoms"))
	TArray<FName> DemonSymptoms;
};

USTRUCT(BlueprintType)
struct FTutorialNewspaperData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FText Description;
};

USTRUCT(BlueprintType)
struct FNewspaperData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int InfectionRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UTexture2D> VillageImage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FText Description;
};