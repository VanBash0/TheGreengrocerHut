#include "Book/Widget/BookPageBase.h"
#include "Components/WidgetSwitcher.h"
#include "Components/RetainerBox.h"

void UBookPageBase::NativeConstruct()
{
    Super::NativeConstruct();
}

void UBookPageBase::Initialize_Implementation(const int32& InPageIndex)
{
    PageIndex = InPageIndex - 1;

    if (PageIndex == -1)
    {
        InitializeChapter();
    }
    else
    {
        InitializePage();
    }
}

void UBookPageBase::Render_Implementation()
{
    if (RetainerBox)
    {
        RetainerBox->RequestRender();
    }
}

void UBookPageBase::ClearPage_Implementation()
{
    if (WidgetSwitcher)
    {
        WidgetSwitcher->SetActiveWidgetIndex(0);
    }
}

void UBookPageBase::InitializePage_Implementation()
{
    if (WidgetSwitcher)
    {
        WidgetSwitcher->SetActiveWidgetIndex(0);
    }

    if (!UpdatePage())
    {
        ClearPage();
    }

    Render();
}

void UBookPageBase::InitializeChapter_Implementation()
{
    if (WidgetSwitcher)
    {
        WidgetSwitcher->SetActiveWidgetIndex(1);
    }

    if (!UpdateChapter())
    {
        ClearPage();
    }

    Render();
}

void UBookPageBase::ShowPage_Implementation()
{
    SetVisibility(ESlateVisibility::Visible);
}

void UBookPageBase::HidePage_Implementation()
{
    SetVisibility(ESlateVisibility::Collapsed);
}