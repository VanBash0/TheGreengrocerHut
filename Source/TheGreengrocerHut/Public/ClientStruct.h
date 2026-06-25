#pragma once
#include "CoreMinimal.h"
#include "ClientStruct.generated.h" 

USTRUCT(BlueprintType)
struct FClient
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FName> Symptoms;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool IsDemon = false;
};

USTRUCT(BlueprintType)
struct FTutorialDay : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FClient> Clients;
};