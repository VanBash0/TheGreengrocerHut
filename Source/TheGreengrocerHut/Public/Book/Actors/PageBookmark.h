#pragma once

#include "CoreMinimal.h"
#include "Animation/SkeletalMeshActor.h"
#include "PageBookmark.generated.h"

class ABook;
class APage;
class UBookPageBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBookmarkHovering, bool, IsHovered);

UCLASS(BlueprintType)
class THEGREENGROCERHUT_API APageBookmark : public ASkeletalMeshActor
{
	GENERATED_BODY()

public:
	APageBookmark();

	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Base")
	void InitializeBookmark(ABook* InOwningBook, TSubclassOf<UBookPageBase> InChapterType);

	UFUNCTION(BlueprintCallable, Category = "Base")
	void InitializeFixedBookmark(ABook* InOwningBook, int32 InTargetPage, float InYOffset);

public:
	UFUNCTION(BlueprintCallable, Category = "Action")
	void AttachToBook();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void AttachToPage(APage* PageToAttaching);

protected:
	UFUNCTION(BlueprintCallable, Category = "Subscribe")
	void OnAttachedPageReleased();

	UFUNCTION(BlueprintCallable, Category = "Subscribe")
	void OnAttachedPageClicked();

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Action")
	void OnBookmarkClicked();
	virtual void OnBookmarkClicked_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Rule")
	bool CanBookmarkHovering();
	virtual bool CanBookmarkHovering_Implementation();

protected:
	UFUNCTION(BlueprintCallable, Category = "Triggers")
	void HandleBeginCursorOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION(BlueprintCallable, Category = "Triggers")
	void HandleEndCursorOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION(BlueprintCallable, Category = "Triggers")
	void HandleCursorClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime")
	TObjectPtr<ABook> OwningBook;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime")
	TObjectPtr<APage> AttachedPage;

public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime")
	bool bIsBookmarkHovering;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime")
	bool bOnRightSide = true;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime")
	int32 TargetPage;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime")
	TSubclassOf<UBookPageBase> ChapterType;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime")
	bool bIsFixedBookmark = false;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime")
	float FixedYOffset = 0.0f;

public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Event")
	FOnBookmarkHovering OnHovering;
};