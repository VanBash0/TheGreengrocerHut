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

    UPROPERTY(Config, EditAnywhere, Category = "Data")
    TSoftObjectPtr<UDataTable> TutorialDaysTable;
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings",
        meta = (DisplayName = "Steepness", ToolTip = "Чем больше значение, тем резче переход к генерации большего числа симптомов",
            ClampMin = "0.0", UIMin = "0.0"))
    float Steepness;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings",
        meta = (DisplayName = "Midpoint", ToolTip = "День, в который достигается временная сложность 50%",
            ClampMin = "1", UIMin = "1"))
    int Midpoint;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings",
        meta = (DisplayName = "Max New Symptoms", ToolTip = "Максимальное число новых симптомов, попадающих в пулл дня",
            ClampMin = "1", UIMin = "1"))
    int MaxNewSymptoms;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings",
        meta = (DisplayName = "Min Clients", ToolTip = "Минимальное количество клиентов в день",
            ClampMin = "1", UIMin = "1"))
    int MinClients;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings",
        meta = (DisplayName = "Max Clients", ToolTip = "Максимальное количество клиентов в день",
            ClampMin = "1", UIMin = "1"))
    int MaxClients;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings",
        meta = (DisplayName = "Max Demons", ToolTip = "Максимальное количество демонов в день",
            ClampMin = "1", UIMin = "1"))
    int MaxDemons;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings",
        meta = (DisplayName = "Min Symptoms", ToolTip = "Минимальное количество симптомов у клиента",
            ClampMin = "1", UIMin = "1"))
    int MinSymptoms;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings",
        meta = (DisplayName = "Max Symptoms", ToolTip = "Максимальное количество симптомов у клиента",
            ClampMin = "1", UIMin = "1"))
    int MaxSymptoms;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings",
        meta = (DisplayName = "Error", ToolTip = "Погрешность (на сколько симптомов может максимум отклониться количество симптомов у пациента/демона от мат. ожидания симптомов у всех пациентов за день)"))
    float Error;
};
