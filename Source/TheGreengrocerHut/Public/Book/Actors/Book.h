#pragma once

#include "CoreMinimal.h"
#include "Animation/SkeletalMeshActor.h"
#include "Book.generated.h"

class APage;

UCLASS(BlueprintType)
class THEGREENGROCERHUT_API ABook : public ASkeletalMeshActor
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Page|Control")
	void NextPage();

	UFUNCTION(BlueprintCallable, Category = "Page|Control")
	void PreviousPage();

	UFUNCTION(BlueprintCallable, Category = "Page|Pool")
	void UpdateWindow();

public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Page|Pool")
	TArray<TObjectPtr<APage>> PagePool;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Page|Pool")
	TArray<TObjectPtr<APage>> ShowedPages;
};
