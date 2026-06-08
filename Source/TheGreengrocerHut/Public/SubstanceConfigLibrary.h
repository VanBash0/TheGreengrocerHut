#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SubstanceConfigLibrary.generated.h"

USTRUCT(BlueprintType)
struct FSubstanceTextureMapping
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mapping")
    FName MaterialParameterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mapping")
    FString SubstanceOutputName;
};

USTRUCT(BlueprintType)
struct FSubstanceMaterialConfig : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    UMaterialInterface* ParentMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    TArray<FSubstanceTextureMapping> TextureMappings;
};

UCLASS()
class USubstanceConfigLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Substance|Config")
    static void AutoGenerateMappings(UPARAM(ref) FSubstanceMaterialConfig& Config)
    {
        Config.TextureMappings.Empty();

        if (!Config.ParentMaterial) return;

        TArray<FMaterialParameterInfo> TextureParams;
        TArray<FGuid> TextureIds;

        UMaterialInstanceDynamic* TempMat = UMaterialInstanceDynamic::Create(Config.ParentMaterial, nullptr);
        if (TempMat)
        {
            TempMat->GetAllTextureParameterInfo(TextureParams, TextureIds);

            for (const FMaterialParameterInfo& Param : TextureParams)
            {
                FSubstanceTextureMapping Mapping;
                Mapping.MaterialParameterName = Param.Name;
                Mapping.SubstanceOutputName = Param.Name.ToString().ToLower();
                Config.TextureMappings.Add(Mapping);
            }

            TempMat->MarkAsGarbage();
        }
    }

    UFUNCTION(BlueprintCallable, Category = "Substance|Config")
    static void AutoGenerateMappingsForSingleRow(UDataTable* DataTable, FName RowName)
    {
        if (!DataTable) return;

        FSubstanceMaterialConfig* Config = DataTable->FindRow<FSubstanceMaterialConfig>(RowName, TEXT(""));
        if (Config)
        {
            AutoGenerateMappings(*Config);

            DataTable->MarkPackageDirty();

            UE_LOG(LogTemp, Log, TEXT("Auto-generated mappings for row: %s"), *RowName.ToString());
        }
    }

    UFUNCTION(BlueprintCallable, Category = "Substance|Config")
    static void AutoGenerateMappingsForDataTable(UDataTable* DataTable)
    {
        if (!DataTable) return;

        TArray<FName> RowNames = DataTable->GetRowNames();

        for (FName RowName : RowNames)
        {
            FSubstanceMaterialConfig* Config = DataTable->FindRow<FSubstanceMaterialConfig>(RowName, TEXT(""));
            if (Config)
            {
                AutoGenerateMappings(*Config);
                UE_LOG(LogTemp, Log, TEXT("Auto-generated mappings for row: %s"), *RowName.ToString());
            }
        }
    }
};
