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
	TArray<FText> DemonSymptoms;
};