#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PageData.generated.h"

class USkeletalMesh;
class UMaterialInterface;
class UAnimMontage;
class UPoseAsset;

UCLASS(BlueprintType)
class THEGREENGROCERHUT_API UPageData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Base")
	TObjectPtr<USkeletalMesh> SM_Page;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Base")
	int32 RandomSeedForSelectMaterial;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Base")
	TArray<TObjectPtr<UMaterialInterface>> MaterialVariantion;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> Page_Take_R;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> Page_Flip_R;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> Page_Take_L;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> Page_Flip_L;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UPoseAsset> PoseForBlendRightAndLeftSide;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Bookmark")
	FName BoneToAttachBookmark;
};