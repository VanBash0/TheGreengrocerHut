#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "StructUtils/InstancedStruct.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"

#define SUBSTANCE_FRAMEWORK_INCLUDED
#include "SubstanceGraphInstance.h"
#include "SympromsStructs.generated.h"

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

inline FVector4 TextureChannelToVector4(ETextureChannel channels)
{
    return FVector4(
        EnumHasAnyFlags(channels, ETextureChannel::R) ? 1.0f : 0.0f,
        EnumHasAnyFlags(channels, ETextureChannel::G) ? 1.0f : 0.0f, 
        EnumHasAnyFlags(channels, ETextureChannel::B) ? 1.0f : 0.0f, 
        EnumHasAnyFlags(channels, ETextureChannel::A) ? 1.0f : 0.0f
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
struct FBodyPart : public FTableRowBase
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Type", MakeStructureDefaultValue = "NewEnumerator0"))
    EBodyPart Type;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Mesh", MakeStructureDefaultValue = "None"))
    TObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "RGB_Mask", MakeStructureDefaultValue = "None"))
    TObjectPtr<UTexture2D> RGB_Mask;
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
    ETextureChannel LayerChannel = ETextureChannel::R;
};

USTRUCT(BlueprintType)
struct FVisualDeformation : public FVisualBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBodyPart OverrideBody;
};

USTRUCT(BlueprintType)
struct FSymptomRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Type", MakeStructureDefaultValue = "NewEnumerator0"))
    EBodyPart Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExcludeBaseStruct))
    TInstancedStruct<FVisualBase> Visual;
};