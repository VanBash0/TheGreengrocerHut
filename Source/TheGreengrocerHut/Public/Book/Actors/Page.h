#pragma once

#include "CoreMinimal.h"
#include "Animation/SkeletalMeshActor.h"
#include "Page.generated.h"

class ABook;
class UPageData;
class UWidgetComponent;
class UMaterialInstanceDynamic;
class UPrimitiveComponent;
struct FKey;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClick);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHovering, bool, IsHovered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRelease);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFinishFlip);

UCLASS(Blueprintable, BlueprintType)
class THEGREENGROCERHUT_API APage : public ASkeletalMeshActor
{
	GENERATED_BODY()

public:
	APage();

	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Base|Main")
	void InitializePage(ABook* InOwnerBook, UPageData* InPageData, int32 InPageNumber, int32 InPageIndex, bool bInIsRightSide, TSubclassOf<UBookPageBase> InWidgetPageR, TSubclassOf<UBookPageBase> InWidgetPageL);

protected:
	UFUNCTION(BlueprintCallable, Category = "Base|Main")
	void InitializeWidgets(TSubclassOf<UBookPageBase> InWidgetPageR, TSubclassOf<UBookPageBase> InWidgetPageL);

public:
	UFUNCTION(BlueprintCallable, Category = "Base|Main")
	void ReleasePage();

	UFUNCTION(BlueprintCallable, Category = "Base|Main")
	void PageActionOnClick();

	UFUNCTION(BlueprintCallable, Category = "Base|Main")
	void OnFinish();

public:
	UFUNCTION(BlueprintCallable, Category = "Base|Actions")
	void FlipPage();

	UFUNCTION(BlueprintCallable, Category = "Base|Actions")
	void HoverPage();

	UFUNCTION(BlueprintCallable, Category = "Base|Actions")
	void UnHoverPage();

	UFUNCTION(BlueprintCallable, Category = "Base|Actions")
	void CheckHovering();

public:
	UFUNCTION(BlueprintCallable, Category = "Base|Helpers")
	void ShowPage();

	UFUNCTION(BlueprintCallable, Category = "Base|Helpers")
	void HidePage();

	UFUNCTION(BlueprintCallable, Category = "Base|Helpers")
	void SetPageActive(bool bActiveClicked);

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UWidgetComponent> Back;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UWidgetComponent> Front;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Flags")
	bool bOnRightSide = true;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Flags")
	bool bIsHover = false;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Flags")
	bool bIsFlippingProcess = false;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	TObjectPtr<UPageData> PageData;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime")
	TObjectPtr<ABook> OwnerBook;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime")
	int32 PageNumber = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime")
	int32 PageIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime")
	TObjectPtr<UMaterialInstanceDynamic> MID_Page;

public:
	UPROPERTY(BlueprintAssignable, Category = "Default")
	FOnClick OnClick;

	UPROPERTY(BlueprintAssignable, Category = "Default")
	FOnHovering OnHovering;

	UPROPERTY(BlueprintAssignable, Category = "Default")
	FOnRelease OnRelease;

	UPROPERTY(BlueprintAssignable, Category = "Default")
	FOnFinishFlip OnFinishFlip;

private:
	UFUNCTION()
	void HandleClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	UFUNCTION()
	void HandleBeginCursorOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void HandleEndCursorOver(UPrimitiveComponent* TouchedComponent);
};