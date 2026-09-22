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
	void InitializePage(ABook* InOwnerBook, UPageData* InPageData, int32 InPageNumber, bool bInIsRightSide);

	UFUNCTION(BlueprintCallable, Category = "Base|Main")
	void InitializeWidgets(int32 PageIndex, TSubclassOf<UBookPageBase> InWidgetPageR, TSubclassOf<UBookPageBase> InWidgetPageL);

public:
	UFUNCTION(BlueprintCallable, Category = "Base|Main")
	void ReleasePage();

	UFUNCTION(BlueprintCallable, Category = "Base|Main")
	bool PageActionOnClick();

	UFUNCTION(BlueprintCallable, Category = "Base|Main")
	void OnFinish();

public:
	UFUNCTION(BlueprintCallable, Category = "Base|Actions")
	bool FlipPage();

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

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime")
	int32 WidgetInitGeneration = 0;

public:
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnClick OnClick;

	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnHovering OnHovering;

	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnRelease OnRelease;

	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnFinishFlip OnFinishFlip;

private:
	UFUNCTION()
	void HandleClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	UFUNCTION()
	void HandleBeginCursorOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void HandleEndCursorOver(UPrimitiveComponent* TouchedComponent);
};