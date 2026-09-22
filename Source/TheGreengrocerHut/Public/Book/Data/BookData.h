#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "Book/Data/PageData.h"
#include "Book/Data/CoverData.h"
#include "Book/Widget/BookPageBase.h"

#include "BookData.generated.h"

class UMaterialInterface;

USTRUCT(BlueprintType)
struct FChapterInfo
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Base")
	TSubclassOf<UBookPageBase> PageType;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Base")
	bool NeedCreateBookmark;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Bookmark", meta = (EditCondition = "NeedCreateBookmark", EditConditionHides))
	TObjectPtr<UMaterialInterface> BookmarkMaterial;
};

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

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Pages")
	TArray<FChapterInfo> PagesInfo;
};
