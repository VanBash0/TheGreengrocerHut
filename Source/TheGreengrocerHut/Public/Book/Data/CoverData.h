#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CoverData.generated.h"

class USkeletalMesh;
class UAnimMontage;
class UAnimSequence;
class UPoseAsset;

UCLASS(BlueprintType)
class THEGREENGROCERHUT_API UCoverData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Base")
	TObjectPtr<USkeletalMesh> SM_Cover;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AM_Open_Front;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AM_Close_Back;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimSequence> AS_Fade_Cover;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UPoseAsset> PoseCloseRightSide;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UPoseAsset> PoseCloseLeftSide;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Detect")
	FName BoneToDetect_Side_R;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Detect")
	FName BoneToDetect_Side_L;
};
