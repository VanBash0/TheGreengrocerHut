#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "StructUtils/InstancedStruct.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "SympromsStructs.generated.h"

USTRUCT(BlueprintType)
struct FVisualBase
{
    GENERATED_BODY()
};

UENUM(BlueprintType)
enum class EMaterialParamType : uint8
{
    Scalar,
    Vector
};

UENUM(BlueprintType)
enum class ETextureChannel : uint8
{
    R,
    G,
    B,
    A
};

USTRUCT(BlueprintType)
struct FMaterialParam
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ParamName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMaterialParamType Type = EMaterialParamType::Scalar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ScalarValue = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor VectorValue = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UTexture> TextureValue = nullptr;
};

USTRUCT(BlueprintType)
struct FVisualOverlay : public FVisualBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UMaterialInterface> Material = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETextureChannel LayerChannel = ETextureChannel::R;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMaterialParam> Params;
};

USTRUCT(BlueprintType)
struct FVisualDeformation : public FVisualBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UStaticMesh> ReplacementMesh = nullptr;
};

USTRUCT(BlueprintType)
struct FS_SymptomRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UObject> HealIngredient = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExcludeBaseStruct))
    TInstancedStruct<FVisualBase> Visual;
};