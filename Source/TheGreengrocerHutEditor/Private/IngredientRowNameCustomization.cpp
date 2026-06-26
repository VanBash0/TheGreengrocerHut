#include "IngredientRowNameCustomization.h"
#include "DetailWidgetRow.h"
#include "GameSettings.h"
#include "Engine/DataTable.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "IngredientStructures.h"

TSharedRef<IPropertyTypeCustomization> FIngredientRowNameCustomization::MakeInstance()
{
    return MakeShareable(new FIngredientRowNameCustomization());
}

void FIngredientRowNameCustomization::CustomizeHeader(
    TSharedRef<IPropertyHandle> PropertyHandle,
    FDetailWidgetRow& HeaderRow,
    IPropertyTypeCustomizationUtils& CustomizationUtils)
{
    RowNameHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FIngredientRowNameRef, RowName));

    RefreshOptions();

    HeaderRow
        .NameContent()
        [
            PropertyHandle->CreatePropertyNameWidget()
        ]
        .ValueContent()
        .MinDesiredWidth(200.f)
        [
            SNew(SComboBox<TSharedPtr<FName>>)
                .OptionsSource(&Options)
                .OnGenerateWidget(this, &FIngredientRowNameCustomization::GenerateOptionWidget)
                .OnSelectionChanged(this, &FIngredientRowNameCustomization::OnSelectionChanged)
                .Content()
                [
                    SNew(STextBlock)
                        .Text(this, &FIngredientRowNameCustomization::GetCurrentValueText)
                ]
        ];
}

void FIngredientRowNameCustomization::RefreshOptions()
{
    Options.Empty();
    Options.Add(MakeShareable(new FName(NAME_None)));

    const UGameProjectSettings* ProjectSettings = GetDefault<UGameProjectSettings>();
    if (ProjectSettings)
    {
        if (ProjectSettings->IngredientTable)
        {
            for (FName RowName : ProjectSettings->IngredientTable->GetRowNames())
            {
                Options.Add(MakeShareable(new FName(RowName)));
            }
        }
    }
}

TSharedRef<SWidget> FIngredientRowNameCustomization::GenerateOptionWidget(TSharedPtr<FName> Item)
{
    return SNew(STextBlock).Text(FText::FromName(Item.IsValid() ? *Item : NAME_None));
}

void FIngredientRowNameCustomization::OnSelectionChanged(TSharedPtr<FName> Item, ESelectInfo::Type SelectInfo)
{
    if (Item.IsValid() && RowNameHandle.IsValid())
    {
        RowNameHandle->SetValue(*Item);
    }
}

FText FIngredientRowNameCustomization::GetCurrentValueText() const
{
    FName CurrentValue;
    if (RowNameHandle.IsValid())
    {
        RowNameHandle->GetValue(CurrentValue);
    }
    return FText::FromName(CurrentValue);
}