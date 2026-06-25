#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ClientStruct.h"
#include "SymptomStructures.h"
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

USTRUCT(BlueprintType)
struct FSymptomWithWeightsData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EBodyPart BodyPart = EBodyPart::None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Weight = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float DemonWeight = 1.0f;
};

USTRUCT(BlueprintType)
struct FGameMetrics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TMap<FName, FSymptomWithWeightsData> SymptomMetrics;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int MaxClientSymptomCount;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool HasDemonPrevious;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int DayNumber;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float HealingFactor;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float KillingFactor;
};

UCLASS(BlueprintType)
class USaveGameData : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GameData")
	TArray<FDaySnapshot> DaySnapshots;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GameData")
	FGameMetrics LastDayMetrics;
};
