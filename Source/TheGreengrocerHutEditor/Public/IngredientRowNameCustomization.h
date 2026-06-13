#pragma once

#include "IPropertyTypeCustomization.h"

class FIngredientRowNameCustomization : public IPropertyTypeCustomization
{
public:
    static TSharedRef<IPropertyTypeCustomization> MakeInstance();

    virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle,
        FDetailWidgetRow& HeaderRow,
        IPropertyTypeCustomizationUtils& CustomizationUtils) override;

    virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle,
        IDetailChildrenBuilder& ChildBuilder,
        IPropertyTypeCustomizationUtils& CustomizationUtils) override {
    }

private:
    TSharedPtr<IPropertyHandle> RowNameHandle;
    TArray<TSharedPtr<FName>> Options;

    void RefreshOptions();
    TSharedRef<SWidget> GenerateOptionWidget(TSharedPtr<FName> Item);
    void OnSelectionChanged(TSharedPtr<FName> Item, ESelectInfo::Type SelectInfo);
    FText GetCurrentValueText() const;
};