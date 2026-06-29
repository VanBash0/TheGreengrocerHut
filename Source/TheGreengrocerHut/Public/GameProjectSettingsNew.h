#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/DataTable.h"
#include "GameSettings.h"
#include "GameProjectSettingsNew.generated.h"

UCLASS(Config = Game, meta = (DisplayName = "Game Settings", SectionName = "TheGreengrocerHutSettings"))
class UGameProjectSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UPROPERTY(Config, EditAnywhere, Category = "Data")
    TSoftObjectPtr<UGameSettings> GameSettingsAsset;

    UPROPERTY(Config, EditAnywhere, Category = "Data")
    TSoftObjectPtr<UDataTable> SymptomTable;

    UPROPERTY(Config, EditAnywhere, Category = "Data")
    TSoftObjectPtr<UDataTable> IngredientTable;

    UPROPERTY(Config, EditAnywhere, Category = "Data")
    TSoftObjectPtr<UDataTable> DefaultBodyPartTable;

    UPROPERTY(Config, EditAnywhere, Category = "Data")
    TSoftObjectPtr<UDataTable> TutorialDaysTable;

    UPROPERTY(Config, EditAnywhere, Category = "Data")
    FString ConverterFolderPath;

    UPROPERTY(Config, EditAnywhere, Category = "Data")
    TSoftObjectPtr<UDataTable> TutorialNewspaperDataTable;

    UPROPERTY(Config, EditAnywhere, Category = "Data")
    TSoftObjectPtr<UDataTable> NewspaperDataTable;
};
