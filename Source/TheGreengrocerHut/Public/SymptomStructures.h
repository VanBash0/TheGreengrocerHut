#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "StructUtils/InstancedStruct.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"

#include "IngredientStructures.h"

#define SUBSTANCE_FRAMEWORK_INCLUDED
#include "SubstanceGraphInstance.h"
#include "SymptomStructures.generated.h"

UENUM(BlueprintType, Meta = (Bitflags = "true", UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ETextureChannel : uint8
{
    None = 0        UMETA(Hidden),
    R = 1 << 0,
    G = 1 << 1,
    B = 1 << 2,
    A = 1 << 3
};
ENUM_CLASS_FLAGS(ETextureChannel)

inline FVector4 TextureChannelToVector4(const TArray<ETextureChannel>& channels)
{
    uint8 mask = 0;
    for (ETextureChannel c : channels) { mask |= (uint8)c; }

    return FVector4(
        (mask & 1) ? 1.0f : 0.0f,
        (mask & 2) ? 1.0f : 0.0f,
        (mask & 4) ? 1.0f : 0.0f,
        (mask & 8) ? 1.0f : 0.0f
    );
}

UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EBodyPart : uint8
{
    None UMETA(Hidden),
    Arm,
    Leg,
    Body,
    Eyes,
    Mouth,
    Ears,
    Nose,
    Hair,
    Face,
    MAX UMETA(Hidden)
};
ENUM_CLASS_FLAGS(EBodyPart)

USTRUCT(BlueprintType)
struct FBodyPartViewData : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Mesh", MakeStructureDefaultValue = "None"))
    TObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "RGB_Mask", MakeStructureDefaultValue = "None"))
    TObjectPtr<UTexture2D> RGB_Mask;
};

USTRUCT(BlueprintType)
struct FDefaultBodyPart : public FBodyPartViewData
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Type", MakeStructureDefaultValue = "NewEnumerator0"))
    EBodyPart Type;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TObjectPtr<UTexture2D> Icon;
};

USTRUCT(BlueprintType)
struct FVisualBase
{
    GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FVisualOverlay : public FVisualBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UMaterialInterface> Material;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<USubstanceGraphInstance> SubstanceGraph = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<ETextureChannel> LayerChannel = { ETextureChannel::R };
};

USTRUCT(BlueprintType)
struct FVisualDeformation : public FVisualBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBodyPartViewData OverrideBody;
};

USTRUCT(BlueprintType)
struct FSymptomRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Type", MakeStructureDefaultValue = "NewEnumerator0"))
    EBodyPart Type;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FText Name;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FText Description;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TObjectPtr<UTexture2D> Icon;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FIngredientRowNameRef IngredientRow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExcludeBaseStruct))
    TInstancedStruct<FVisualBase> Visual;
};