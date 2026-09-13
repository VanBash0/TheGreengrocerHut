#pragma once

#include "CoreMinimal.h"
#include "Animation/SkeletalMeshActor.h"

#include "Book/Data/PageData.h"
#include "Book/Widget/BookPageBase.h"

#include "Page.generated.h"

UCLASS(BlueprintType)
class THEGREENGROCERHUT_API APage : public ASkeletalMeshActor
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Base|Main")
	void Initialize();
};
