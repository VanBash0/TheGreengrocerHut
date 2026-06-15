#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DeveloperSettings.h"
#include "GameSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Game Settings"))
class UGameProjectSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UPROPERTY(Config, EditAnywhere, Category = "Data")
    TSoftObjectPtr<UGameSettings> GameSettingsAsset;

    UPROPERTY(Config, EditAnywhere, Category = "Data")
    TSoftObjectPtr<UDataTable> SymptomTable;

    UPROPERTY(Config, EditAnywhere, Category = "Data")
    TSoftObjectPtr<UDataTable> DefaultBodyPartTable;
};

USTRUCT(BlueprintType)
struct FPriorityData
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Priority")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Priority")
    TObjectPtr<UTexture2D> LabelTexture;
};

UCLASS(BlueprintType, Blueprintable)
class UGameSettings : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredient/Priority")
    TMap<int32, FPriorityData> IngredientPriorityData;
};
