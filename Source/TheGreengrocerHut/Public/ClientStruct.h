#pragma once
#include "CoreMinimal.h"
#include "ClientStruct.generated.h" 

USTRUCT(BlueprintType)
struct FClient
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    TArray<FName> Symptoms;

    UPROPERTY(BlueprintReadWrite)
    bool IsDemon = false;
};