#pragma once

#include "CoreMinimal.h"
#include "TermsOfUse.generated.h"

USTRUCT(BlueprintType)
struct FTermsOfUse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1", ClampMax = "2", UIMin = "-1", UIMax = "2"))
    int AddPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "10", UIMin = "1", UIMax = "10"))
    int Mixing;
};