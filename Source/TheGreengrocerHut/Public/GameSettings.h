#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "IngredientStructures.h"
#include "ClientStruct.h"
#include "GameSettings.generated.h"

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
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredients|Priority",
        meta = (DisplayName = "Ingredient Priority Data", ToolTip = "Численное значение приоритетов добавления ингредиентов"))
    TMap<int32, FPriorityData> IngredientPriorityData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings|Maths",
        meta = (DisplayName = "Steepness", ToolTip = "Чем больше значение, тем резче переход к генерации большего числа симптомов",
            ClampMin = "0.0", UIMin = "0.0"))
    float Steepness;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings|Maths",
        meta = (DisplayName = "Midpoint", ToolTip = "День, в который достигается временная сложность 50%",
            ClampMin = "1", UIMin = "1"))
    int Midpoint;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings|Symptoms",
        meta = (DisplayName = "Min New Symptoms", ToolTip = "Минимальное число новых симптомов, попадающих в пулл дня",
            ClampMin = "1", UIMin = "1"))
    int MinNewSymptoms;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings|Symptoms",
        meta = (DisplayName = "Max New Symptoms", ToolTip = "Максимальное число новых симптомов, попадающих в пулл дня",
            ClampMin = "1", UIMin = "1"))
    int MaxNewSymptoms;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings|Clients",
        meta = (DisplayName = "Min Clients", ToolTip = "Минимальное количество клиентов в день",
            ClampMin = "1", UIMin = "1"))
    int MinClients;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings|Clients",
        meta = (DisplayName = "Max Clients", ToolTip = "Максимальное количество клиентов в день",
            ClampMin = "1", UIMin = "1"))
    int MaxClients;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings|Clients",
        meta = (DisplayName = "Max Demons", ToolTip = "Максимальное количество демонов в день",
            ClampMin = "1", UIMin = "1"))
    int MaxDemons;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings|Symptoms",
        meta = (DisplayName = "Min Symptoms", ToolTip = "Минимальное количество симптомов у клиента",
            ClampMin = "1", UIMin = "1"))
    int MinSymptoms;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings|Symptoms",
        meta = (DisplayName = "Max Symptoms", ToolTip = "Максимальное количество симптомов у клиента",
            ClampMin = "1", UIMin = "1"))
    int MaxSymptoms;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings|Symptoms",
        meta = (DisplayName = "Error", ToolTip = "Погрешность (на сколько симптомов может максимум отклониться количество симптомов у пациента/демона от мат. ожидания симптомов у всех пациентов за день)"))
    float Error;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings|Symptoms|Weights",
        meta = (DisplayName = "Weight Recovery Rate", ToolTip = "Величина, на которую восстанавливается вес симптома, если он не выпадает",
            ClampMin = "0.0", UIMin = "0.0"))
    float WeightRecoveryRate;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings|Symptoms|Weights",
        meta = (DisplayName = "Weight Min Value", ToolTip = "Величина, до которой обнуляется вес симптома при его выборе",
            ClampMin = "0.0", UIMin = "0.0"))
    float WeightMinValue;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings|Day Cycle",
        meta = (DisplayName = "Wait Time Range", ToolTip = "Интервал времени ожидания нового клиента"))
    FVector2D WaitClientTimeRange;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings|Symptoms|Weights",
        meta = (DisplayName = "Max Weight", ToolTip = "Максимальное значение веса симптома",
            ClampMin = "0.0", UIMin = "0.0"))
    float MaxWeight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings|Symptoms|Weights",
        meta = (DisplayName = "New Symptom Weight", ToolTip = "Вес симптома в день его добавления в пул",
            ClampMin = "0.0", UIMin = "0.0"))
    float NewSymptomWeight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client Generator Settings|Symptoms|Weights",
        meta = (DisplayName = "New Demon Symptom Weight", ToolTip = "Вес симптома для демона в день его добавления в пул",
            ClampMin = "0.0", UIMin = "0.0"))
    float NewDemonSymptomWeight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredients|Base Potions",
        meta = (DisplayName = "Base Potions Map", ToolTip = "Пороги числа симптомов для открытия базовых ингредиентов"))
    TMap<int32, FIngredientRowNameRef> BasePotionsMap;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredients|Base Potions",
        meta = (DisplayName = "Poison Base", ToolTip = "Row Name отравы"))
    FIngredientRowNameRef PoisonBase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredients|Base Potions",
        meta = (DisplayName = "Bottle Base", ToolTip = "Row Name склянки"))
    FIngredientRowNameRef BottleBase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Infection Rate",
        meta = (DisplayName = "Start Healing Factor", ToolTip = "Базовое значение фактора лечения (чем больше, тем быстрее деревня исцеляется)"))
    float StartHealingFactor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Infection Rate",
        meta = (DisplayName = "Start Killing Factor", ToolTip = "Базовое значение фактора калечения (чем больше, тем быстрее деревня заражается)"))
    float StartKillingFactor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Infection Rate",
        meta = (DisplayName = "Delta Healing Factor", ToolTip = "Величина, на которую увеличивается фактор лечения при исцелении пациента/отраве демона"))
    float DeltaHealingFactor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Infection Rate",
        meta = (DisplayName = "Delta Killing Factor", ToolTip = "Величина, на которую увеличивается фактор калечения при неисцелении пациента/неотраве демона"))
    float DeltaKillingFactor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Infection Rate",
        meta = (DisplayName = "Potion Result Delta Map", ToolTip = "Базовая дельта заражения для каждого исхода (EPotionResult)"))
    TMap<EPotionResult, float> PotionResultDeltaMap;
};
