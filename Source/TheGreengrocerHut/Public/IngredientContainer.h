#pragma once

#include "CoreMinimal.h"
#include "IngredientContainer.generated.h"

USTRUCT(BlueprintType)
struct FIngredientContainer : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform DecalTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* SFX;
};
