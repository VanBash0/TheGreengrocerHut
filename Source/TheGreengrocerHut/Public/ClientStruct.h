#pragma once

#include "CoreMinimal.h"
#include "Dialog/GuestDialogStructs.h"
#include "ClientStruct.generated.h" 

USTRUCT(BlueprintType)
struct FClient
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<TObjectPtr<UGuestData>> Guests;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FName> Symptoms;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool IsDemon = false;
};

UENUM(BlueprintType)
enum class EPotionResult : uint8
{
    Human_Healed,
    Human_NotHealed,
    Human_Poisoned,

    Demon_Poisoned,
    Demon_NotPoisoned,
    Demon_GivenGoodPotion
};

USTRUCT(BlueprintType)
struct FPotionCompareResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Potion Compare")
    TArray<FName> GivenIngredients;

    UPROPERTY(BlueprintReadOnly, Category = "Potion Compare")
    TArray<bool> IngredientValidity;

    UPROPERTY(BlueprintReadOnly, Category = "Potion Compare")
    float ValidFraction = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Potion Compare")
    bool IsPoison = false;

    UPROPERTY(BlueprintReadOnly, Category = "Potion Compare")
    TMap<int32, bool> TierMatchResults;
};

USTRUCT(BlueprintType)
struct FClientResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Client Result")
    EPotionResult Result = EPotionResult::Human_NotHealed;

    UPROPERTY(BlueprintReadOnly, Category = "Client Result")
    float DeltaInfectionRate = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Client Result")
    FClient Client;

    UPROPERTY(BlueprintReadOnly, Category = "Client Result")
    TArray<FName> GivenIngredients;

    UPROPERTY(BlueprintReadOnly, Category = "Client Result")
    TArray<bool> IngredientValidity;

    UPROPERTY(BlueprintReadOnly, Category = "Client Result")
    float ValidFraction = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Client Result")
    TMap<int32, bool> TierMatchResults;

    UPROPERTY(BlueprintReadOnly, Category = "Client Result")
    bool IsPotionGood = false;

    FString ToString() const
    {
        TArray<FString> IngredientStrings;
        IngredientStrings.Reserve(GivenIngredients.Num());
        for (const FName& Name : GivenIngredients)
        {
            IngredientStrings.Add(Name.ToString());
        }

        FString IngredientsStr = FString::Join(IngredientStrings, TEXT(", "));

        FString ValidityStr;
        for (bool bValid : IngredientValidity)
        {
            ValidityStr += bValid ? TEXT("1 ") : TEXT("0 ");
        }

        FString TierStr;
        for (const auto& Pair : TierMatchResults)
        {
            TierStr += FString::Printf(TEXT("[%d:%s] "), Pair.Key, Pair.Value ? TEXT("true") : TEXT("false"));
        }

        return FString::Printf(
            TEXT("Result=%s, Delta=%.2f, IsDemon=%s, Ingredients=[%s], Validity=[%s], Score=%.2f, Tiers=[%s], IsGood=%s"),
            *UEnum::GetValueAsString(Result),
            DeltaInfectionRate,
            Client.IsDemon ? TEXT("true") : TEXT("false"),
            *IngredientsStr,
            *ValidityStr,
            ValidFraction,
            *TierStr,
            IsPotionGood ? TEXT("true") : TEXT("false"));
    }
};

USTRUCT(BlueprintType)
struct FTutorialDay : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FClient> Clients;
};