#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ClientStruct.h"
#include "SaveGameData.generated.h"

USTRUCT(BlueprintType)
struct FDaySnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<FName> DaySymptoms;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<FName> DemonSymptoms;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<FClient> DayClients;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float VillageInfectionRate;
};

UCLASS(BlueprintType)
class USaveGameData : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GameData")
	TArray<FDaySnapshot> DaySnapshots;
};
