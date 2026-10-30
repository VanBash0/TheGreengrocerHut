#pragma once

#include "CoreMinimal.h"
#include "Animation/SkeletalMeshActor.h"
#include "NewspaperRoll.generated.h"

class UUserWidget;
class UWidgetComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS(Blueprintable, BlueprintType)
class THEGREENGROCERHUT_API ANewspaperRoll : public ASkeletalMeshActor
{
	GENERATED_BODY()

public:
	ANewspaperRoll();

	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UWidgetComponent> WidgetCom;

	UPROPERTY(EditAnywhere, Category = "Settings")
	TSubclassOf<UUserWidget> NewspaperWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Settings")
	TArray<TObjectPtr<UMaterialInterface>> NewspaperMaterials;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Runtime")
	TObjectPtr<UMaterialInstanceDynamic> MID_Newspaper;
};
