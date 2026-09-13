#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "PageData.h"
#include "CoverData.h"

#include "BookData.generated.h"

UCLASS(BlueprintType)
class THEGREENGROCERHUT_API UBookData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Base")
	TObjectPtr<UCoverData> Cover;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Base")
	TArray<TObjectPtr<UPageData>> Pages;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Base")
	int32 RandomSeedForPageTypeSelector;
};
