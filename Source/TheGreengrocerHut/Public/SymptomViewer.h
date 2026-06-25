#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "SymptomStructures.h"
#include "ClientStruct.h"
#include "GameSettings.h"
#include "SubstanceCoreHelpers.h"
#include "SymptomViewer.generated.h"

struct FVisualOverlayPoolEntry
{
    bool bIsInUse = false;

    UStaticMeshComponent* MeshComponent = nullptr;
    UMaterialInstanceDynamic* DynamicMaterial = nullptr;

    USubstanceGraphInstance* SubstanceInstance = nullptr;
};

struct FBodyPartData
{
    UStaticMeshComponent* BaseMeshComp = nullptr;
    TArray<FVisualOverlayPoolEntry*> OverlayEntries;

    inline void Hide()
    {
        BaseMeshComp->SetVisibility(false, true);
    }

    inline void Show()
    {
        BaseMeshComp->SetVisibility(true, true);
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRenderComplete);

UCLASS(BlueprintType, Blueprintable)
class THEGREENGROCERHUT_API ASymptomViewer : public AActor
{
	GENERATED_BODY()
public:
    ASymptomViewer();

    virtual void BeginPlay() override;
    virtual void BeginDestroy() override;

public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Symptom Viewer")
    void SetNewSymptoms(const FClient& newClient);
    virtual void SetNewSymptoms_Implementation(const FClient& newClient);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Symptom Viewer")
    void ShowBodyPart(const EBodyPart& PartType);
    virtual void ShowBodyPart_Implementation(const EBodyPart& PartType);

    UFUNCTION(BlueprintCallable, Category = "Symptom Viewer")
    void Reset();

    UFUNCTION(BlueprintCallable, Category = "Symptom Viewer", BlueprintPure)
    bool IsRendering() const { return _toRender.Num() > 0; }

public:
    UPROPERTY(BlueprintReadOnly, Category = "Settings")
    TObjectPtr<UDataTable> SymptomsTable;

    UPROPERTY(BlueprintReadOnly, Category = "Settings")
    TObjectPtr<UDataTable> DefaultBodyPartTable;

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Symptom Viewer")
    USceneComponent* SymptomRoot;

    UPROPERTY(BlueprintAssignable, Category = "Symptom Viewer|Events", meta = (DisplayName = "On Render Complete", ToolTip = "Fires when all Substance textures are fully rendered and ready"))
    FOnRenderComplete OnRenderComplete;

private:
    void InitializeViewer();

    FVisualOverlayPoolEntry* GetPoolEntry(UStaticMeshComponent* Root, UMaterialInterface* Material);
    USubstanceGraphInstance* CopyGraphAndSetMaterial(USubstanceGraphInstance* Graph, UMaterialInterface* MainMaterial, UMaterialInstanceDynamic* DimMaterial);
    const std::pair<std::pair<bool, FVisualDeformation>, TArray<FVisualOverlay>> SelectBodySymptomsByType(const TArray<FSymptomRow>& Symptoms);

    TMap<EBodyPart, FBodyPartData> _bodyParts;
    TArray<FVisualOverlayPoolEntry> _pool;

private:
    void RenderTick();

    FTimerHandle _renderTimerHandle;
    TArray<USubstanceGraphInstance*> _toRender;
};
