#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Book.generated.h"

class APage;
class APageBookmark;
class UBookData;
class UPageData;
class UBookPageBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPageChanged, int32, PrevPageN, int32, CurPageN);

USTRUCT(BlueprintType)
struct FChapterRuntimeInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Base")
	int32 PageCount = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Base")
	int32 PadedPageCount = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Base")
	int32 StartIndex = 0;
};

UCLASS(BlueprintType)
class THEGREENGROCERHUT_API ABook : public AActor
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

	UFUNCTION(BlueprintCallable, Category = "Base")
	void ComputeTotalPageCount();

	UFUNCTION(BlueprintCallable, Category = "Base")
	void CreateBookmarks();

	UFUNCTION(BlueprintCallable, Category = "Base")
	void CreateExitBookmark();

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

protected:
	UFUNCTION(BlueprintPure, Category = "Page|Control")
	int32 GetNextPageNumber(int32 From) const;

	UFUNCTION(BlueprintPure, Category = "Page|Control")
	int32 GetPreviousPageNumber(int32 From) const;

	void SetCurrentPage(int32 NewPage);

	UFUNCTION(BlueprintCallable, Category = "Page|Control")
	void WaitForCoverThenFlip();

	void StartFlippingSequence();

public:
	UFUNCTION(BlueprintPure, Category = "Page|Metrics")
	void PageFlipProgress(float& Progress);

public:
	UFUNCTION(BlueprintPure, Category = "Page|Pool|Helpers")
	UPageData* GetPageInitializeData(int32 PageN);

	UFUNCTION(BlueprintPure, Category = "Page|Pool|Helpers")
	void GetPageWidgetData(int32 PageN, int32& PageIndex, TSubclassOf<UBookPageBase>& Widget_R, TSubclassOf<UBookPageBase>& Widget_L);

	UFUNCTION(BlueprintPure, Category = "Page|Pool|Helpers")
	bool ShouldFullyInitializePage(int32 PageN) const;

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
	void TrimPagePool();

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
	void TryFinishFlipSequence();

	UFUNCTION(BlueprintCallable, Category = "Page|Chapter")
	FVector GetBookmarkAttachedLocation(TSubclassOf<UBookPageBase> ChapterType);

	UFUNCTION(BlueprintPure, Category = "Page|Chapter")
	FVector GetBookmarkLocationForPage(int32 StartPage, float YOffset) const;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Component")
	TObjectPtr<USkeletalMeshComponent> MeshComp;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Component")
	TObjectPtr<USceneComponent> PageRoot;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Component")
	TObjectPtr<UBoxComponent> BookCollision;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Base")
	TObjectPtr<UBookData> BookData;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Base")
	TSubclassOf<APage> PageClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Base")
	TSubclassOf<APageBookmark> BookmarkClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Visual")
	int32 DefaultWindowSize;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Visual")
	FVector PageOffsetDirRight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Visual")
	FVector PageOffsetDirLeft;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Visual")
	float RootOffsetClosed = -1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Visual")
	float RootOffsetOpened = 3.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Visual")
	float DefaultOffsetSpeed = 7.5f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Visual")
	float NearPagesOffsetSpeed = 7.5f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|PageFlipping")
	int32 FlippingShowInitializedPages;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|PageFlipping")
	float FlipOverlapDelay = 0.12f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|PageFlipping")
	int32 FlippingWindowSize = 10;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Pool")
	int32 MaxPooledPages = 12;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Bookmark")
	float BookmarkYSpawnOffset = -25.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Bookmark")
	FVector2D BookmarkZInterval = FVector2D(0, -20.0f);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Bookmark")
	TSubclassOf<APageBookmark> ExitBookmarkClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Bookmark")
	float ExitBookmarkYOffset = 0.0f;

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
	int32 SequenceStartPage;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime|PageFlipping")
	bool bIsFlippingSequenceActive;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime|PageFlipping")
	int32 TargetAnimationPagesTotal;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime|PageFlipping")
	int32 PreSequenceWindowSize;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime|PageFlipping")
	FTimerHandle FlipOverlapTimerHandle;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime|PageFlipping")
	FTimerHandle FlipSettleTimerHandle;

public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Page|Pool")
	TArray<TObjectPtr<APage>> PagePool;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Page|Pool")
	TArray<TObjectPtr<APage>> ShowedPages;

public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Page|Fade")
	FTimerHandle OffsetTimerHandler;

public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Page|Chapter")
	TMap<TSubclassOf<UBookPageBase>, FChapterRuntimeInfo> ChapterMetrics;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Page|Chapter")
	TMap<TSubclassOf<UBookPageBase>, TObjectPtr<APageBookmark>> Bookmarks;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Page|Chapter")
	TObjectPtr<APageBookmark> ExitBookmark;

public:
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly, Category = "Default")
	FOnPageChanged OnPageChanged;
};