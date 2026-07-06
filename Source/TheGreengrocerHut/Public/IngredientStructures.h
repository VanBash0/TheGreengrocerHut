#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "IngredientStructures.generated.h"

USTRUCT(BlueprintType)
struct FIngredientContainerVariation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform DecalTransform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<USoundCue> SFX;
};

UCLASS(BlueprintType)
class UIngredientContainer : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FIngredientContainerVariation> Variations;
};

USTRUCT(BlueprintType)
struct FTermsOfUse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1", ClampMax = "2", UIMin = "-1", UIMax = "2"))
	int AddPriority;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "10", UIMin = "1", UIMax = "10"))
	int Mixing;
};

USTRUCT(BlueprintType)
struct FIngredient : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Ingredient")
    FText DisplayName;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Ingredient", meta = (MultiLine = "true"))
    FText Description;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Ingredient")
    FLinearColor MainColor;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Ingredient")
    TObjectPtr<UTexture2D> Icon;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Ingredient")
    TObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Ingredient")
    TObjectPtr<USoundCue> SFX;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Ingredient")
    TObjectPtr<UIngredientContainer> Container;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Ingredient")
    FTermsOfUse TermsOfUse;
};

USTRUCT(BlueprintType)
struct FIngredientRowNameRef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RowName;

    bool operator==(const FIngredientRowNameRef& Other) const
    {
        return RowName == Other.RowName;
    }
};

USTRUCT(BlueprintType)
struct FIngredientContainerSave
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText FirstIngredientName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 IngredientsAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName IngredientRowName;
};