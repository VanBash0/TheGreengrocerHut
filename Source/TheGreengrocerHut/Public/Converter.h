#pragma once

#include "CoreMinimal.h"
#include "ConverterRecipe.h"
#include "Converter.generated.h"

UCLASS(Blueprintable, BlueprintType)
class THEGREENGROCERHUT_API UConverter : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText Description;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UTexture2D> Icon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<USkeletalMesh> Mesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UAnimMontage> ActiveMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UAnimMontage> UseMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<USoundCue> SFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FTransform TargetPoint;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FVector BoxExtent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<FConverterRecipe> RecipeArray;

    UFUNCTION(BlueprintPure)
    bool ContainsRecipe(FName IngredientRowName);

    UFUNCTION(BlueprintPure)
    FConverterRecipe GetRecipe(FName IngredientRowName);
};