#include "TheGreengrocerHutEditor.h"
#include "IngredientRowNameCustomization.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FTheGreengrocerHutEditorModule"

void FTheGreengrocerHutEditorModule::StartupModule()
{
    UE_LOG(LogTemp, Warning, TEXT("TheGreengrocerHutEditor: StartupModule called"));

    FPropertyEditorModule& PropertyModule =
        FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    PropertyModule.RegisterCustomPropertyTypeLayout(
        "IngredientRowNameRef",
        FOnGetPropertyTypeCustomizationInstance::CreateStatic(
            &FIngredientRowNameCustomization::MakeInstance)
    );

    UE_LOG(LogTemp, Warning, TEXT("TheGreengrocerHutEditor: Registered IngredientRowNameRef customization"));
}

void FTheGreengrocerHutEditorModule::ShutdownModule()
{
    if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
    {
        FPropertyEditorModule& PropertyModule =
            FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyModule.UnregisterCustomPropertyTypeLayout("IngredientRowNameRef");
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FTheGreengrocerHutEditorModule, TheGreengrocerHutEditor)