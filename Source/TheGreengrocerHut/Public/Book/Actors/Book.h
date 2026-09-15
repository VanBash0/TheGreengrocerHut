#pragma once

#include "CoreMinimal.h"
#include "Animation/SkeletalMeshActor.h"
#include "Book.generated.h"

class APage;
class UBookData;
class UPageData;
class UBookPageBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPageChanged, int32, PrevPageN, int32, CurPageN);

UCLASS(BlueprintType)
class THEGREENGROCERHUT_API ABook : public ASkeletalMeshActor
{
	GENERATED_BODY()

public:
	ABook();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Base")
	void OnBookCliked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

public:
	UFUNCTION(BlueprintCallable, Category = "Base")
	void InitializeBook();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Base")
	void OnOpenBook();
	void OnOpenBook_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Base")
	void OnCloseBook();
	void OnCloseBook_Implementation();

public:
	UFUNCTION(BlueprintCallable, Category = "Page|Control")
	void NextPage();

	UFUNCTION(BlueprintCallable, Category = "Page|Control")
	void PreviousPage();

	UFUNCTION(BlueprintCallable, Category = "Page|Control")
	void GoToPage(int32 TargetPage);

public:
	UFUNCTION(BlueprintPure, Category = "Page|Metrics")
	void PageFlipProgress(float& Progress);

public:
	UFUNCTION(BlueprintPure, Category = "Page|Pool|Helpers")
	UPageData* GetPageInitializeData(int32 PageN);

	UFUNCTION(BlueprintPure, Category = "Page|Pool|Helpers")
	void GetPageWidgetData(int32 PageN, int32& PageIndex, TSubclassOf<UBookPageBase>& Widget_R, TSubclassOf<UBookPageBase>& Widget_L);

	UFUNCTION(BlueprintPure, Category = "Page|Pool|Helpers")
	bool IsPageNumberShowed(int32 PageN);

	UFUNCTION(BlueprintCallable, Category = "Page|Pool|Helpers")
	APage* GetPageByNumber(int32 PageN);
	UFUNCTION(BlueprintCallable, Category = "Page|Pool")
	void GetOrCreatePage(int32 PageNumber, APage*& Page, bool bFullInit = true);

	UFUNCTION(BlueprintCallable, Category = "Page|Pool")
	bool TryGetFromPool(int32 PageN, APage*& Page);

	UFUNCTION(BlueprintCallable, Category = "Page|Pool")
	void CreateNewPage(int32 PageN, APage*& Page);

	UFUNCTION(BlueprintCallable, Category = "Page|Pool")
	void InitializePage(APage*& Page, int32 PageNumber, bool bFullInit = true);

	UFUNCTION(BlueprintCallable, Category = "Page|Pool")
	void ReleaseAllPage();

	UFUNCTION(BlueprintCallable, Category = "Page|Pool")
	void UpdateWindow();

	UFUNCTION(BlueprintCallable, Category = "Page|Pool")
	void ReleaseOutOfWindowPages(int32 WindowMin, int32 WindowMax, bool IgnoreAnimation = false);

	UFUNCTION(BlueprintCallable, Category = "Page|Pool")
	void CreateMissingWindowPages(int32 WindowMin, int32 WindowMax);

	UFUNCTION(BlueprintCallable, Category = "Page|Pool")
	void UpdatePagesActivation();

public:
	UFUNCTION(BlueprintPure, Category = "Page|Fade")
	FVector GetPageOffset(int32 PageN);

	UFUNCTION(BlueprintCallable, Category = "Page|Fade")
	void UpdatePageRoot();

	UFUNCTION(BlueprintCallable, Category = "Page|Fade")
	void StartOffsetPageProcess();

	UFUNCTION(BlueprintCallable, Category = "Page|Fade")
	void UpdateOffsetPageProcess();

	UFUNCTION(BlueprintCallable, Category = "Page|Fade")
	void StopOffsetPageProcess();

	UFUNCTION(BlueprintCallable, Category = "Page|Fade")
	void FlipToTargetPageProcess();

	UFUNCTION(BlueprintCallable, Category = "Page|Fade")
	void HandleLastPageFlipped();

public:
	UFUNCTION(BlueprintCallable, Category = "Page|Event")
	void OnPageStartFlipping();

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Component")
	TObjectPtr<USceneComponent> PageRoot;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Base")
	TObjectPtr<UBookData> BookData;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Base")
	TSubclassOf<APage> PageClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Visual")
	int32 DefaultWindowSize;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Visual")
	FVector PageOffsetDirRight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Visual")
	FVector PageOffsetDirLeft;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Visual")
	TSubclassOf<UBookPageBase> Widget;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Settings|PageFlipping")
	int32 FlippingShowInitializedPages;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Runtime|Flags")
	bool bCanFlipPage;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Runtime|PageControl")
	int32 CurPageNumber = -1;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime|PageControl")
	int32 PrevPageNumber;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime|PageControl")
	int32 CurrentWindowSize;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime|PageControl")
	int32 TotalPageCount;

public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime|PageFlipping")
	int32 FinalTargetPage;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime|PageFlipping")
	bool bIsFlippingSequenceActive;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime|PageFlipping")
	TArray<int32> TargetAnimationPages;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime|PageFlipping")
	int32 TargetAnimationPagesTotal;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime|PageFlipping")
	TObjectPtr<APage> LastFlippedPage;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime|PageFlipping")
	FTimerHandle FlipSequenceTimerHandle;

public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Page|Pool")
	TArray<TObjectPtr<APage>> PagePool;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Page|Pool")
	TArray<TObjectPtr<APage>> ShowedPages;

public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Page|Fade")
	FTimerHandle OffsetTimerHandler;

public:
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly, Category = "Default")
	FOnPageChanged OnPageChanged;
};
