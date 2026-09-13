#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BookPageBase.generated.h"

class UWidgetSwitcher;
class URetainerBox;
class UNamedSlot;

UCLASS(Abstract, Blueprintable, BlueprintType)
class THEGREENGROCERHUT_API UBookPageBase : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Base|Pipline")
	void ClearPage();
	virtual void ClearPage_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Base|Pipline")
	void Render();
	virtual void Render_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Base|Page")
	void InitializePage();
	virtual void InitializePage_Implementation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Base|Page")
	bool UpdatePage();

	UFUNCTION(BlueprintNativeEvent, Category = "Base|Chapter")
	void InitializeChapter();
	virtual void InitializeChapter_Implementation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Base|Chapter")
	bool UpdateChapter();

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Base|Pipline")
	void Initialize(const int32& InPageIndex);
	virtual void Initialize_Implementation(const int32& InPageIndex);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Base|Control")
	void ShowPage();
	virtual void ShowPage_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Base|Control")
	void HidePage();
	virtual void HidePage_Implementation();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Base|Metrics")
	int32 GetPageCount();

public:
	UPROPERTY(BlueprintReadOnly, Category = "Base|Page")
	int32 PageIndex = INDEX_NONE;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<URetainerBox> RetainerBox;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UNamedSlot> DataSlot;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UNamedSlot> ChapterSlot;
};
