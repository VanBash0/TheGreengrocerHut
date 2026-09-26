#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Symptom/SymptomStructures.h"
#include "ClientStruct.h"
#include "GameProjectSettings.h"
#include "GameSettings.h"
#include "SubstanceCoreHelpers.h"
#include "Engine/StreamableManager.h"
#include "SymptomViewer.generated.h"

USTRUCT()
struct FVisualOverlayPoolEntry
{
    GENERATED_BODY()

    bool bIsInUse = false;

    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> MeshComponent = nullptr;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial = nullptr;

    UPROPERTY()
    TObjectPtr<USubstanceGraphInstance> SubstanceInstance = nullptr;
};

USTRUCT()
struct FBodyPartData
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> BaseMeshComp = nullptr;

    TArray<FVisualOverlayPoolEntry*> OverlayEntries;

    inline void Hide() { BaseMeshComp->SetVisibility(false, true); }
    inline void Show() { BaseMeshComp->SetVisibility(true, true); }
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
    TObjectPtr<USceneComponent> SymptomRoot;

    UPROPERTY(BlueprintAssignable, Category = "Symptom Viewer|Events", meta = (DisplayName = "On Render Complete", ToolTip = "Fires when all Substance textures are fully rendered and ready"))
    FOnRenderComplete OnRenderComplete;

private:
    void InitializeViewer();

    FVisualOverlayPoolEntry* GetPoolEntry(UStaticMeshComponent* Root, UMaterialInterface* Material);

    USubstanceGraphInstance* CopyGraphAndSetMaterial(USubstanceGraphInstance* graph, UMaterialInterface* mainMaterial, UMaterialInstanceDynamic* dimMaterial);

    const std::pair<std::pair<bool, FVisualDeformation>, TArray<FVisualOverlay>> SelectBodySymptomsByType(const TArray<FSymptomRow>& Symptoms);

    struct FPendingSymptomBodyPart
    {
        EBodyPart PartType = EBodyPart::None;
        TSoftObjectPtr<UStaticMesh> Mesh;
        TSoftObjectPtr<UTexture2D> Mask;
        TArray<FVisualOverlay> Overlays;
    };

    void OnSettingsTablesLoaded(TSoftObjectPtr<UDataTable> SymptomTableSoft, TSoftObjectPtr<UDataTable> DefaultBodyPartTableSoft);

    void FinishSetNewSymptoms(TArray<FPendingSymptomBodyPart> PendingParts, uint32 RequestId);

    UPROPERTY()
    TMap<EBodyPart, FBodyPartData> _bodyParts;

    UPROPERTY()
    TArray<FVisualOverlayPoolEntry> _pool;

private:
    void RenderTick();

    FTimerHandle _renderTimerHandle;

    UPROPERTY()
    TArray<TObjectPtr<USubstanceGraphInstance>> _toRender;

private:
    static void ReleaseSubstanceGraphInstance(USubstanceGraphInstance* Instance);

    TSharedPtr<FStreamableHandle> _tablesStreamableHandle;
    TSharedPtr<FStreamableHandle> _streamableHandle;
    uint32 _renderRequestId = 0;
};