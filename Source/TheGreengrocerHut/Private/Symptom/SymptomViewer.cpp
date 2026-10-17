#include "Symptom/SymptomViewer.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "SubstanceGraphInstance.h"
#include "SubstanceOutputData.h"
#include "SubstanceCoreHelpers.h"
#include "Engine/AssetManager.h"

//CORE METHOD
ASymptomViewer::ASymptomViewer()
{
    PrimaryActorTick.bCanEverTick = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ROOT"));

    SymptomRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ROOT_SYMPTOM"));
    SymptomRoot->SetupAttachment(RootComponent);
    SymptomRoot->SetRelativeLocation(FVector::ZeroVector);
    SymptomRoot->SetRelativeRotation(FRotator::ZeroRotator);

    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Constructor called for %s"), *GetName());
}

void ASymptomViewer::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] BeginPlay start"));

    const UGameProjectSettings* ProjectSettings = GetDefault<UGameProjectSettings>();
    if (!ProjectSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] UGameProjectSettings::GetDefault returned NULL!"));
        return;
    }

    TSoftObjectPtr<UDataTable> SymptomTableSoft = ProjectSettings->SymptomTable;
    TSoftObjectPtr<UDataTable> DefaultBodyPartTableSoft = ProjectSettings->DefaultBodyPartTable;

    TArray<FSoftObjectPath> toLoad;
    if (!SymptomTableSoft.IsNull()) { toLoad.AddUnique(SymptomTableSoft.ToSoftObjectPath()); }
    if (!DefaultBodyPartTableSoft.IsNull()) { toLoad.AddUnique(DefaultBodyPartTableSoft.ToSoftObjectPath()); }

    if (toLoad.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] SymptomTable / DefaultBodyPartTable are not set on UGameProjectSettings! Aborting BeginPlay."));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Requesting async load of %d setting table(s)"), toLoad.Num());

    _tablesStreamableHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(toLoad,
        FStreamableDelegate::CreateUObject(this, &ASymptomViewer::OnSettingsTablesLoaded, SymptomTableSoft, DefaultBodyPartTableSoft));

    if (!_tablesStreamableHandle.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] RequestAsyncLoad for settings tables returned an invalid handle!"));
    }

    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] BeginPlay end (tables streaming in async)"));
}

//LOGIC METHOD
void ASymptomViewer::OnSettingsTablesLoaded(TSoftObjectPtr<UDataTable> SymptomTableSoft, TSoftObjectPtr<UDataTable> DefaultBodyPartTableSoft)
{
    _tablesStreamableHandle.Reset();

    SymptomsTable = SymptomTableSoft.Get();
    DefaultBodyPartTable = DefaultBodyPartTableSoft.Get();

    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] SymptomsTable loaded: %s (%s)"),
        SymptomsTable ? TEXT("OK") : TEXT("NULL"),
        SymptomsTable ? *SymptomsTable->GetPathName() : TEXT("n/a"));

    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] DefaultBodyPartTable loaded: %s (%s)"),
        DefaultBodyPartTable ? TEXT("OK") : TEXT("NULL"),
        DefaultBodyPartTable ? *DefaultBodyPartTable->GetPathName() : TEXT("n/a"));

    if (!SymptomsTable || !DefaultBodyPartTable)
    {
        UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] Tables not assigned after async load! Aborting."));
        return;
    }

    InitializeViewer();
}

void ASymptomViewer::BeginDestroy()
{
    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] BeginDestroy start, pool size=%d"), _pool.Num());

    ++_renderRequestId;

    if (_tablesStreamableHandle.IsValid())
    {
        _tablesStreamableHandle->CancelHandle();
        _tablesStreamableHandle.Reset();
    }

    if (_streamableHandle.IsValid())
    {
        _streamableHandle->CancelHandle();
        _streamableHandle.Reset();
    }

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(_renderTimerHandle);
    }

    for (FVisualOverlayPoolEntry& entry : _pool)
    {
        if (entry.SubstanceInstance)
        {
            ReleaseSubstanceGraphInstance(entry.SubstanceInstance);
            entry.SubstanceInstance->ConditionalBeginDestroy();
            entry.SubstanceInstance = nullptr;
        }

        if (entry.MeshComponent)
        {
            entry.MeshComponent->DestroyComponent();
            entry.MeshComponent = nullptr;
        }
    }

    for (auto& kv : _bodyParts)
    {
        if (kv.Value.BaseMeshComp)
        {
            kv.Value.BaseMeshComp->DestroyComponent();
            kv.Value.BaseMeshComp = nullptr;
        }
    }

    _bodyParts.Empty();
    _pool.Empty();
    _toRender.Empty();

    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] BeginDestroy end"));

    Super::BeginDestroy();
}

void ASymptomViewer::ReleaseSubstanceGraphInstance(USubstanceGraphInstance* Instance)
{
    if (!Instance)
    {
        return;
    }

    for (auto& OutputPair : Instance->OutputInstances)
    {
        USubstanceOutputData* OutputData = OutputPair.Value;
        if (!OutputData)
        {
            continue;
        }

        if (UTexture2D* OutputTexture = Cast<UTexture2D>(OutputData->GetData()))
        {
            UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] ReleaseSubstanceGraphInstance: clearing RF_Standalone/RF_Public on texture %s and marking it garbage"),
                *GetNameSafe(OutputTexture));

            OutputTexture->ClearFlags(RF_Standalone | RF_Public);
            OutputTexture->MarkAsGarbage();
        }

        OutputData->ClearFlags(RF_Standalone | RF_Public);
        OutputData->MarkAsGarbage();
    }

    Instance->ClearFlags(RF_Standalone | RF_Public);
}

//LOGIC METHOD
void ASymptomViewer::InitializeViewer()
{
    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] InitializeViewer start"));

    if (!RootComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] RootComponent is null!"));
        return;
    }

    _pool.Reserve(64);

    for (int partIndex = 1; partIndex < (int)EBodyPart::MAX; partIndex++)
    {
        EBodyPart partType = static_cast<EBodyPart>(partIndex);

        FBodyPartData bodyData = {};
        bodyData.BaseMeshComp = NewObject<UStaticMeshComponent>(this);
        bodyData.BaseMeshComp->SetCastShadow(false);
        bodyData.BaseMeshComp->SetupAttachment(SymptomRoot);
        bodyData.BaseMeshComp->RegisterComponent();
        bodyData.BaseMeshComp->SetVisibility(false);

        _bodyParts.Add(partType, bodyData);

        UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Created body part comp for %s (%s)"),
            *StaticEnum<EBodyPart>()->GetNameStringByValue((int32)partType),
            *GetNameSafe(bodyData.BaseMeshComp));
    }

    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Viewer initialized with %d body parts"), _bodyParts.Num());
}

void ASymptomViewer::SetNewSymptoms_Implementation(const FClient& newClient)
{
    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] SetNewSymptoms_Implementation start, %d symptoms in client"), newClient.Symptoms.Num());

    Reset();

    if (!SymptomsTable)
    {
        UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] Symptoms table not assigned!"));
        return;
    }

    if (!DefaultBodyPartTable)
    {
        UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] Body parts table not assigned!"));
        return;
    }

    FString ContextString = TEXT("Getting Symptom Data");
    TMap<EBodyPart, TArray<FSymptomRow>> symptomsByPart;

    for (const FName& name : newClient.Symptoms)
    {
        if (FSymptomRow* symptom = SymptomsTable->FindRow<FSymptomRow>(name, ContextString))
        {
            symptomsByPart.FindOrAdd(symptom->Type).Add(*symptom);
            UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Symptom '%s' mapped to body part %s"),
                *name.ToString(),
                *StaticEnum<EBodyPart>()->GetNameStringByValue((int32)symptom->Type));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] Symptom row '%s' NOT FOUND in SymptomsTable!"), *name.ToString());
        }
    }

    TArray<FPendingSymptomBodyPart> pendingParts;
    TArray<FSoftObjectPath> toLoad;

    for (int partIndex = 1; partIndex < (int)EBodyPart::MAX; partIndex++)
    {
        EBodyPart partType = static_cast<EBodyPart>(partIndex);

        TArray<FSymptomRow>* partSymptoms = symptomsByPart.Find(partType);
        auto visual = SelectBodySymptomsByType(partSymptoms ? *partSymptoms : TArray<FSymptomRow>());

        FPendingSymptomBodyPart pending;
        pending.PartType = partType;
        pending.Overlays = visual.second;

        if (visual.first.first)
        {
            pending.Mesh = visual.first.second.OverrideBody.Mesh;
            pending.Mask = visual.first.second.OverrideBody.RGB_Mask;

            UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Part %s uses OVERRIDE body mesh=%s mask=%s"),
                *StaticEnum<EBodyPart>()->GetNameStringByValue((int32)partType),
                *pending.Mesh.ToString(), *pending.Mask.ToString());
        }
        else
        {
            FName RowName = FName(StaticEnum<EBodyPart>()->GetNameStringByValue((int32)partType));
            FDefaultBodyPart* tableRow = DefaultBodyPartTable->FindRow<FDefaultBodyPart>(RowName, TEXT("GetBodyPart"));
            if (tableRow)
            {
                pending.Mesh = tableRow->Mesh;
                pending.Mask = tableRow->RGB_Mask;
                UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Part %s uses DEFAULT body mesh=%s mask=%s"),
                    *StaticEnum<EBodyPart>()->GetNameStringByValue((int32)partType),
                    *pending.Mesh.ToString(), *pending.Mask.ToString());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] DefaultBodyPartTable row '%s' NOT FOUND!"), *RowName.ToString());
            }
        }

        if (!pending.Mesh.IsNull()) { toLoad.AddUnique(pending.Mesh.ToSoftObjectPath()); }
        if (!pending.Mask.IsNull()) { toLoad.AddUnique(pending.Mask.ToSoftObjectPath()); }

        for (const FVisualOverlay& overlay : pending.Overlays)
        {
            if (!overlay.Material.IsNull()) { toLoad.AddUnique(overlay.Material.ToSoftObjectPath()); }
            if (!overlay.SubstanceGraph.IsNull()) { toLoad.AddUnique(overlay.SubstanceGraph.ToSoftObjectPath()); }
        }

        pendingParts.Add(MoveTemp(pending));
    }

    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Requesting async load of %d soft asset(s) across %d body part(s)"),
        toLoad.Num(), pendingParts.Num());

    const uint32 thisRequestId = _renderRequestId;

    if (toLoad.Num() == 0)
    {
        FinishSetNewSymptoms(MoveTemp(pendingParts), thisRequestId);
        return;
    }

    _streamableHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(toLoad,
        FStreamableDelegate::CreateUObject(this, &ASymptomViewer::FinishSetNewSymptoms, MoveTemp(pendingParts), thisRequestId));

    if (!_streamableHandle.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] RequestAsyncLoad returned an invalid handle!"));
    }

    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] SetNewSymptoms_Implementation end (assets streaming in async)"));
}

void ASymptomViewer::FinishSetNewSymptoms(TArray<FPendingSymptomBodyPart> PendingParts, uint32 RequestId)
{
    if (RequestId != _renderRequestId)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] FinishSetNewSymptoms: stale request (id=%u, current=%u), ignoring"),
            RequestId, _renderRequestId);
        return;
    }

    _streamableHandle.Reset();

    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] FinishSetNewSymptoms: assets loaded, building %d body part(s)"), PendingParts.Num());

    _toRender.Empty();

    for (FPendingSymptomBodyPart& part : PendingParts)
    {
        FBodyPartData* bodyData = &_bodyParts.FindOrAdd(part.PartType);

        UStaticMesh* meshToUse = part.Mesh.Get();
        UTexture2D* maskToUse = part.Mask.Get();

        bodyData->BaseMeshComp->SetStaticMesh(meshToUse);

        UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Part %s has %d overlays to place, StaticMesh set: %s"),
            *StaticEnum<EBodyPart>()->GetNameStringByValue((int32)part.PartType),
            part.Overlays.Num(),
            meshToUse ? TEXT("YES") : TEXT("NULL"));

        for (const FVisualOverlay& overlay : part.Overlays)
        {
            UMaterialInterface* material = overlay.Material.Get();
            USubstanceGraphInstance* substanceGraph = overlay.SubstanceGraph.Get();

            FVisualOverlayPoolEntry* element = GetPoolEntry(bodyData->BaseMeshComp, material);

            if (!element || !element->MeshComponent)
            {
                UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] GetPoolEntry returned NULL entry or NULL MeshComponent for part %s!"),
                    *StaticEnum<EBodyPart>()->GetNameStringByValue((int32)part.PartType));
                continue;
            }

            UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Overlay entry acquired: comp=%s material=%s visible=%s attachedTo=%s"),
                *GetNameSafe(element->MeshComponent),
                *GetNameSafe(material),
                element->MeshComponent->IsVisible() ? TEXT("true") : TEXT("false"),
                *GetNameSafe(element->MeshComponent->GetAttachParent()));

            if (element->DynamicMaterial)
            {
                element->DynamicMaterial->SetVectorParameterValue(FName(TEXT("Channel")), overlay.LayerChannel);
                element->DynamicMaterial->SetTextureParameterValue(FName(TEXT("Mask")), maskToUse);
                UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] DynamicMaterial params set on %s (Mask=%s)"),
                    *GetNameSafe(element->DynamicMaterial), *GetNameSafe(maskToUse));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] element->DynamicMaterial is NULL, overlay will show base material or nothing!"));
            }

            bodyData->OverlayEntries.Add(element);

            if (!substanceGraph)
            {
                UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Overlay has no SubstanceGraph, skipping substance render for this entry"));
                continue;
            }

            USubstanceGraphInstance* newGraph = CopyGraphAndSetMaterial(substanceGraph, material, element->DynamicMaterial);

            if (newGraph)
            {
                _toRender.Add(newGraph);
                element->SubstanceInstance = newGraph;
                UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Queued substance graph %s for render (toRender count now %d)"),
                    *GetNameSafe(newGraph), _toRender.Num());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] CopyGraphAndSetMaterial returned NULL for overlay on part %s!"),
                    *StaticEnum<EBodyPart>()->GetNameStringByValue((int32)part.PartType));
            }
        }

        bodyData->Hide();
    }

    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Finished building overlays. Total graphs to render: %d"), _toRender.Num());

    if (_toRender.Num() > 0)
    {
        TArray<SubstanceAir::GraphInstanceSPtr> toAsync;
        for (const auto& g : _toRender)
        {
            bool bInstanceValid = g && g->Instance.get() != nullptr;
            UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Preparing RenderAsync: graph=%s Instance valid=%s (raw ptr=%p)"),
                *GetNameSafe(g), bInstanceValid ? TEXT("true") : TEXT("false"), g ? g->Instance.get() : nullptr);

            if (bInstanceValid)
            {
                toAsync.Add(g->Instance);
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Calling Substance::Helpers::RenderAsync with %d graphs (out of %d in _toRender)"), toAsync.Num(), _toRender.Num());
        Substance::Helpers::RenderAsync(toAsync);

        if (GetWorld())
        {
            FTimerDelegate Del;
            Del.BindLambda([this]() { this->RenderTick(); });
            GetWorld()->GetTimerManager().SetTimer(_renderTimerHandle, Del, 0.1f, true);
            UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] RenderTick timer started"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] GetWorld() is NULL, cannot start RenderTick timer!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] No graphs to render, broadcasting OnRenderComplete immediately"));
        OnRenderComplete.Broadcast();
    }

    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] FinishSetNewSymptoms end"));
}

void ASymptomViewer::ShowBodyPart_Implementation(const EBodyPart& PartType)
{
    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] ShowBodyPart_Implementation called for %s, IsRendering=%s"),
        *StaticEnum<EBodyPart>()->GetNameStringByValue((int32)PartType),
        IsRendering() ? TEXT("true") : TEXT("false"));

    if (IsRendering())
    {
        UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] ShowBodyPart ABORTED because IsRendering() is true (_toRender.Num()=%d) - part will NOT be shown!"), _toRender.Num());
        return;
    }

    for (auto& kv : _bodyParts)
    {
        if (kv.Key == PartType)
        {
            kv.Value.Show();
            UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Showing part %s, BaseMesh visible=%s, overlay count=%d"),
                *StaticEnum<EBodyPart>()->GetNameStringByValue((int32)kv.Key),
                kv.Value.BaseMeshComp->IsVisible() ? TEXT("true") : TEXT("false"),
                kv.Value.OverlayEntries.Num());

            for (FVisualOverlayPoolEntry* entry : kv.Value.OverlayEntries)
            {
                if (entry && entry->MeshComponent)
                {
                    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer]   Overlay comp=%s visible=%s material=%s mesh=%s"),
                        *GetNameSafe(entry->MeshComponent),
                        entry->MeshComponent->IsVisible() ? TEXT("true") : TEXT("false"),
                        *GetNameSafe(entry->DynamicMaterial),
                        *GetNameSafe(entry->MeshComponent->GetStaticMesh()));
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("[SymptomViewer]   Overlay entry or its MeshComponent is NULL!"));
                }
            }
        }
        else
        {
            kv.Value.Hide();
        }
    }
}

//POOL HELPER
USubstanceGraphInstance* ASymptomViewer::CopyGraphAndSetMaterial(USubstanceGraphInstance* graph, UMaterialInterface* mainMaterial, UMaterialInstanceDynamic* dimMaterial)
{
    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] CopyGraphAndSetMaterial start, graph=%s mainMaterial=%s"),
        *GetNameSafe(graph), *GetNameSafe(mainMaterial));

    USubstanceGraphInstance* newGraph = graph->Duplicate();

    if (!newGraph)
    {
        UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] graph->Duplicate() returned NULL!"));
        return nullptr;
    }

    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] After Duplicate(): newGraph=%s, source graph Instance valid=%s, newGraph Instance valid=%s"),
        *GetNameSafe(newGraph),
        (graph->Instance.get() != nullptr) ? TEXT("true") : TEXT("false"),
        (newGraph->Instance.get() != nullptr) ? TEXT("true") : TEXT("false"));

    newGraph->ConditionalPostLoad();
    newGraph->SetInputInt("$outputsize", TArray<int32>{ 10, 10 });
    newGraph->SetInputInt("$randomseed", TArray<int32>{  FMath::RandRange(0, 100000) });
    newGraph->CreateOutputs();

    TArray<FString> outputNames = newGraph->GetOutputNames();
    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] newGraph has %d outputs"), outputNames.Num());
    for (const FString& outputName : outputNames)
    {
        newGraph->EnableOutput(outputName, true);
    }

    for (auto& OutputPair : newGraph->OutputInstances)
    {
        USubstanceOutputData* OutputData = OutputPair.Value;
        UTexture2D* OutputTexture = OutputData ? Cast<UTexture2D>(OutputData->GetData()) : nullptr;
        if (OutputTexture)
        {
            OutputTexture->UpdateResource();
            UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Manually called UpdateResource() on %s"), *GetNameSafe(OutputTexture));
        }
    }

    {
        int32 dbgIndex = 0;
        for (auto& OutputPair : newGraph->OutputInstances)
        {
            USubstanceOutputData* OutputData = OutputPair.Value;
            UTexture2D* OutputTexture = OutputData ? Cast<UTexture2D>(OutputData->GetData()) : nullptr;

            UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Output[%d] right after CreateOutputs: OutputData=%p Texture=%p Path='%s'"),
                dbgIndex,
                OutputData,
                OutputTexture,
                OutputTexture ? *OutputTexture->GetPathName() : TEXT("<null>"));

            dbgIndex++;
        }
    }

    TMap<FName, int32> mapping;
    if (mainMaterial && graph)
    {
        TArray<FMaterialParameterInfo> textureParams;
        TArray<FGuid> textureGuids;
        mainMaterial->GetAllTextureParameterInfo(textureParams, textureGuids);

        UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] mainMaterial has %d texture params"), textureParams.Num());

        for (const FMaterialParameterInfo& param : textureParams)
        {
            UTexture* TextureValue = nullptr;
            mainMaterial->GetTextureParameterValue(param, TextureValue);

            if (!TextureValue) { continue; }

            int32 index = 0;
            for (auto& OutputPair : graph->OutputInstances)
            {
                USubstanceOutputData* OutputData = OutputPair.Value;
                if (!OutputData) { index++; continue; }

                UTexture2D* OutputTexture = Cast<UTexture2D>(OutputData->GetData());
                if (!OutputTexture || OutputTexture != TextureValue) { index++; continue; }

                SubstanceAir::OutputInstance* OutputInstance = Substance::Helpers::GetSubstanceOutputByID(graph, OutputPair.Key);
                FString DebugIdentifier = OutputInstance ? FString(OutputInstance->mDesc.mIdentifier.c_str()) : TEXT("<null OutputInstance>");

                mapping.Add(param.Name, index);

                UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Mapped material param '%s' -> substance output index %d (identifier='%s', for reference only)"),
                    *param.Name.ToString(), index, *DebugIdentifier);

                break;
            }
        }
    }

    if (mapping.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] No texture param mapping found! dimMaterial will keep default/empty textures."));
        return newGraph;
    }

    for (auto& map : mapping)
    {
        FName ParamName = map.Key;
        int32 TargetIndex = map.Value;

        bool bAssigned = false;
        int32 index = 0;

        for (auto& OutputPair : newGraph->OutputInstances)
        {
            if (index != TargetIndex) { index++; continue; }

            USubstanceOutputData* OutputData = OutputPair.Value;
            UTexture2D* OutputTexture = OutputData ? Cast<UTexture2D>(OutputData->GetData()) : nullptr;

            if (!OutputTexture)
            {
                UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] Output at index %d has no valid UTexture2D yet (param '%s')"), TargetIndex, *ParamName.ToString());
                break;
            }

            dimMaterial->SetTextureParameterValue(ParamName, OutputTexture);
            bAssigned = true;

            UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Assigned new output texture ptr=%p path='%s' (index %d) to param '%s' on %s"),
                OutputTexture, *OutputTexture->GetPathName(), TargetIndex, *ParamName.ToString(), *GetNameSafe(dimMaterial));

            break;
        }

        if (!bAssigned)
        {
            UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] Could not find matching new output at index %d (param '%s')"),
                TargetIndex, *ParamName.ToString());
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] CopyGraphAndSetMaterial end, returning %s"), *GetNameSafe(newGraph));

    return newGraph;
}

const std::pair<std::pair<bool, FVisualDeformation>, TArray<FVisualOverlay>> ASymptomViewer::SelectBodySymptomsByType(const TArray<FSymptomRow>& Symptoms)
{
    bool deformExist = false;
    FVisualDeformation deform;

    TArray<FVisualOverlay> overlays;

    for (const FSymptomRow& s : Symptoms)
    {
        if (const FVisualOverlay* Overlay = s.Visual.GetPtr<FVisualOverlay>())
        {
            overlays.Add(*Overlay);
        }
        else if (!deformExist)
        {
            if (const FVisualDeformation* Deform = s.Visual.GetPtr<FVisualDeformation>())
            {
                deformExist = true;
                deform = *Deform;
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] SelectBodySymptomsByType: %d symptoms in, %d overlays out, deformExist=%s"),
        Symptoms.Num(), overlays.Num(), deformExist ? TEXT("true") : TEXT("false"));

    return { { deformExist, deform }, overlays };
}

void ASymptomViewer::Reset()
{
    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Reset start, pool size=%d"), _pool.Num());

    ++_renderRequestId;

    if (_streamableHandle.IsValid())
    {
        _streamableHandle->CancelHandle();
        _streamableHandle.Reset();
    }

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(_renderTimerHandle);
    }

    int32 FreedCount = 0;
    for (FVisualOverlayPoolEntry& entry : _pool)
    {
        if (entry.bIsInUse)
        {
            entry.bIsInUse = false;
            FreedCount++;
        }

        if (entry.DynamicMaterial)
        {
            entry.DynamicMaterial->MarkAsGarbage();
            entry.DynamicMaterial = nullptr;
        }

        if (entry.MeshComponent)
        {
            entry.MeshComponent->SetVisibility(false);
            entry.MeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
        }

        if (entry.SubstanceInstance)
        {
            ReleaseSubstanceGraphInstance(entry.SubstanceInstance);
            entry.SubstanceInstance->MarkAsGarbage();
            entry.SubstanceInstance = nullptr;
        }
    }

    for (auto& kv : _bodyParts)
    {
        kv.Value.OverlayEntries.Empty();
    }

    _toRender.Empty();

    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] Reset end, freed %d pool entries"), FreedCount);
}

FVisualOverlayPoolEntry* ASymptomViewer::GetPoolEntry(UStaticMeshComponent* root, UMaterialInterface* material)
{
    for (FVisualOverlayPoolEntry& entry : _pool)
    {
        if (!entry.bIsInUse)
        {
            entry.bIsInUse = true;

            if (material && entry.MeshComponent)
            {
                entry.DynamicMaterial = UMaterialInstanceDynamic::Create(material, this, FName("OverlayMaterial"));
                entry.MeshComponent->SetMaterial(0, entry.DynamicMaterial);
            }

            if (root && entry.MeshComponent)
            {
                entry.MeshComponent->AttachToComponent(root, FAttachmentTransformRules::SnapToTargetIncludingScale);
                entry.MeshComponent->SetStaticMesh(root->GetStaticMesh());
                entry.MeshComponent->SetVisibility(true, true);
            }

            UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] GetPoolEntry: REUSED entry, comp=%s attachedTo=%s visible=%s"),
                *GetNameSafe(entry.MeshComponent), *GetNameSafe(root),
                entry.MeshComponent ? (entry.MeshComponent->IsVisible() ? TEXT("true") : TEXT("false")) : TEXT("n/a"));

            return &entry;
        }
    }

    FVisualOverlayPoolEntry newEntry;
    newEntry.bIsInUse = true;

    newEntry.MeshComponent = NewObject<UStaticMeshComponent>(this);
    newEntry.MeshComponent->SetCastShadow(false);
    newEntry.MeshComponent->SetVisibility(false);

    if (root)
    {
        newEntry.MeshComponent->SetupAttachment(root);
        newEntry.MeshComponent->SetStaticMesh(root->GetStaticMesh());
    }
    newEntry.MeshComponent->RegisterComponent();
    newEntry.MeshComponent->SetVisibility(true, true);

    if (material)
    {
        newEntry.DynamicMaterial = UMaterialInstanceDynamic::Create(material, this, FName("OverlayMaterial"));
        newEntry.MeshComponent->SetMaterial(0, newEntry.DynamicMaterial);
    }

    UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] GetPoolEntry: CREATED NEW entry #%d, comp=%s attachedTo=%s"),
        _pool.Num(), *GetNameSafe(newEntry.MeshComponent), *GetNameSafe(root));

    if (_pool.Num() >= _pool.Max())
    {
        UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] WARNING: _pool is about to grow beyond reserved capacity (%d)! Raw pointers previously returned from GetPoolEntry may become DANGLING after this Add()."), _pool.Max());
    }

    _pool.Add(newEntry);
    return &_pool.Last();
}

//RENDERER
void ASymptomViewer::RenderTick()
{
    auto IsGraphReady = [](USubstanceGraphInstance* graph) -> bool
        {
            if (!graph || !graph->Instance)
            {
                UE_LOG(LogTemp, Error, TEXT("[SymptomViewer] RenderTick: graph or graph->Instance is NULL in _toRender!"));
                return false;
            }

            for (auto& pair : graph->OutputInstances)
            {
                if (!pair.Value) { continue; }

                UTexture2D* tex = Cast<UTexture2D>(pair.Value->GetData());

                UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] RenderTick: graph=%s output ptr=%p path='%s' sizeX=%d"),
                    *GetNameSafe(graph), tex, tex ? *tex->GetPathName() : TEXT("<null>"), tex ? tex->GetSizeX() : -1);

                if (!tex || tex->GetSizeX() != 1024)
                {
                    return false;
                }
            }

            return true;
        };

    const int32 removedCount = _toRender.RemoveAll([&](USubstanceGraphInstance* graph)
        {
            const bool bReady = IsGraphReady(graph);
            UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] RenderTick: graph=%s ready=%s"),
                *GetNameSafe(graph), bReady ? TEXT("true") : TEXT("false"));
            return bReady;
        });

    if (removedCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] RenderTick: %d graph(s) finished, %d remaining"), removedCount, _toRender.Num());
    }

    if (_toRender.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[SymptomViewer] RenderTick: all graphs rendered, stopping timer and broadcasting OnRenderComplete"));

        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(_renderTimerHandle);
        }

        OnRenderComplete.Broadcast();
    }
}