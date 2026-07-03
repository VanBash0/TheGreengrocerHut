#include "DebugFunctionLibrary.h"
#include "Blueprint/UserWidget.h"

void UDebugFunctionLibrary::SafeSetBrushFromTexture(UUserWidget* WidgetContext, UImage* TargetImage, UTexture2D* Texture)
{
    if (!TargetImage) {
        UE_LOG(LogTemp, Warning, TEXT("Передан пустой Target Image!"));
        return;
    }

	if (Texture) {
		TargetImage->SetBrushFromTexture(Texture);
	}
	else {
		FString WidgetName = WidgetContext ? WidgetContext->GetName() : TEXT("Unknown_Widget");
		FString ImageName = TargetImage->GetName();

		FString ErrorMessage = FString::Printf(
			TEXT("CRITICAL UI ERROR: В виджете [%s] попытались установить пустую текстуру (None) в компонент картинки [%s]!"),
			*WidgetName,
			*ImageName
		);

		UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);

#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				10.f,
				FColor::Red,
				FString::Printf(TEXT("🚨 UI ERROR в %s (см. Лог)"), *WidgetName)
			);
		}
#endif
	}
}