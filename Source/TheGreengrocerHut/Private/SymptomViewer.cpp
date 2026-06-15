#include "SymptomViewer.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "SubstanceGraphInstance.h"
#include "SubstanceOutputData.h"
#include "SubstanceCoreHelpers.h"

//CORE METHOD
ASymptomViewer::ASymptomViewer()
{
    PrimaryActorTick.bCanEverTick = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ROOT"));

    SymptomRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ROOT_SYMPTOM"));
    SymptomRoot->SetupAttachment(RootComponent);
    SymptomRoot->SetRelativeLocation(FVector::ZeroVector);
    SymptomRoot->SetRelativeRotation(FRotator::ZeroRotator);

}

void ASymptomViewer::BeginPlay()
{
    Super::BeginPlay();

    const UGameProjectSettings* ProjectSettings = GetDefault<UGameProjectSettings>();
    if (ProjectSettings)
    {
        SymptomsTable = ProjectSettings->SymptomTable.LoadSynchronous();
        DefaultBodyPartTable = ProjectSettings->DefaultBodyPartTable.LoadSynchronous();
    }

    if (!SymptomsTable || !DefaultBodyPartTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Tables not assigned!"));
        return;
    }

    InitializeViewer();
}

void ASymptomViewer::BeginDestroy()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(_renderTimerHandle);
    }

    for (FVisualOverlayPoolEntry& entry : _pool)
    {
        if (entry.SubstanceInstance)
        {
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

    Super::BeginDestroy();
}

//LOGIC METHOD
void ASymptomViewer::InitializeViewer()
{
    if (!RootComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("RootComponent is null!"));
        return;
    }

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
    }

    UE_LOG(LogTemp, Error, TEXT("Viewer initialized with %d body parts"), _bodyParts.Num());
}

void ASymptomViewer::SetNewSymptoms_Implementation(const FClient& newClient)
{
    Reset();

    if (!SymptomsTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Symptoms table not assigned!"));
        return;
    }

    if (!DefaultBodyPartTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Body parts table not assigned!"));
        return;
    }

    FString ContextString = TEXT("Getting Symptom Data");
    TMap<EBodyPart, TArray<FSymptomRow>> symptomsByPart;

    for (const FName& name : newClient.Symptoms)
    {
        if (FSymptomRow* symptom = SymptomsTable->FindRow<FSymptomRow>(name, ContextString))
        {
            symptomsByPart.FindOrAdd(symptom->Type).Add(*symptom);
        }
    }

    _toRender.Empty();

    for (int partIndex = 1; partIndex < (int)EBodyPart::MAX; partIndex++)
    {
        EBodyPart partType = static_cast<EBodyPart>(partIndex);

        FBodyPartData* bodyData = &_bodyParts.FindOrAdd(partType);

        TArray<FSymptomRow>* partSymptoms = symptomsByPart.Find(partType);
        auto visual = SelectBodySymptomsByType(partSymptoms ? *partSymptoms : TArray<FSymptomRow>());

        UStaticMesh* meshToUse = nullptr;
        UTexture2D* maskToUse = nullptr;

        if (visual.first.first)
        {
            meshToUse = visual.first.second.OverrideBody.Mesh;
            maskToUse = visual.first.second.OverrideBody.RGB_Mask;
        }
        else
        {
            FName RowName = FName(StaticEnum<EBodyPart>()->GetNameStringByValue((int32)partType));
            FDefaultBodyPart* tableRow = DefaultBodyPartTable->FindRow<FDefaultBodyPart>(RowName, TEXT("GetBodyPart"));
            if (tableRow)
            {
                meshToUse = tableRow->Mesh;
                maskToUse = tableRow->RGB_Mask;
            }
        }

        bodyData->BaseMeshComp->SetStaticMesh(meshToUse);

        for (const FVisualOverlay& overlay : visual.second)
        {
            FVisualOverlayPoolEntry* element = GetPoolEntry(bodyData->BaseMeshComp, overlay.Material);

            if (!element || !element->MeshComponent) { continue; }

            if (element->DynamicMaterial)
            {
                element->DynamicMaterial->SetVectorParameterValue(FName(TEXT("Channel")), TextureChannelToVector4(overlay.LayerChannel));
                element->DynamicMaterial->SetTextureParameterValue(FName(TEXT("Mask")), maskToUse);
            }

            bodyData->OverlayEntries.Add(element);

            if (!overlay.SubstanceGraph) { continue; }

            USubstanceGraphInstance* newGraph = CopyGraphAndSetMaterial(overlay.SubstanceGraph, overlay.Material, element->DynamicMaterial);

            if (newGraph)
            {
                _toRender.Add(newGraph);
                element->SubstanceInstance = newGraph;
            }
        }

        bodyData->Hide();
    }

    if (_toRender.Num() > 0)
    {
        TArray<SubstanceAir::GraphInstanceSPtr> toAsync;
        for (const auto& g : _toRender) { toAsync.Add(g->Instance); }
        Substance::Helpers::RenderAsync(toAsync);

        if (GetWorld())
        {
            FTimerDelegate Del;
            Del.BindLambda([this]() { this->RenderTick(); });
            GetWorld()->GetTimerManager().SetTimer(_renderTimerHandle, Del, 0.001f, true);
        }
    }
    else
    {
        OnRenderComplete.Broadcast();
    }
}

void ASymptomViewer::ShowBodyPart_Implementation(const EBodyPart& PartType)
{
    if (IsRendering()) { return; }

    for (auto& kv : _bodyParts)
    {
        if (kv.Key == PartType)
        {
            kv.Value.Show();
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
    USubstanceGraphInstance* newGraph = graph->Duplicate();

    if (!newGraph) { return nullptr; }

    newGraph->ConditionalPostLoad();
    newGraph->SetInputInt("$outputsize", TArray<int32>{ 10, 10 });
    newGraph->SetInputInt("$randomseed", TArray<int32>{  FMath::RandRange(0, 100000) });
    newGraph->CreateOutputs();

    TArray<FString> outputNames = newGraph->GetOutputNames();
    for (const FString& outputName : outputNames)
    {
        newGraph->EnableOutput(outputName, true);
    }

    TMap<FName, FString> mapping;
    if (mainMaterial && graph)
    {
        TArray<FMaterialParameterInfo> textureParams;
        TArray<FGuid> textureGuids;
        mainMaterial->GetAllTextureParameterInfo(textureParams, textureGuids);

        for (const FMaterialParameterInfo& param : textureParams)
        {
            UTexture* TextureValue = nullptr;
            mainMaterial->GetTextureParameterValue(param, TextureValue);

            if (!TextureValue) { continue; }

            for (auto& OutputPair : graph->OutputInstances)
            {
                USubstanceOutputData* OutputData = OutputPair.Value;
                if (!OutputData) { continue; }

                UTexture2D* OutputTexture = Cast<UTexture2D>(OutputData->GetData());
                if (!OutputTexture || OutputTexture != TextureValue) { continue; }

                SubstanceAir::OutputInstance* OutputInstance = Substance::Helpers::GetSubstanceOutputByID(graph, OutputPair.Key);

                if (!OutputInstance) { break; }

                FString OutputIdentifier = FString(OutputInstance->mDesc.mIdentifier.c_str());
                mapping.Add(param.Name, OutputIdentifier);

                break;
            }
        }
    }

    if (mapping.Num() == 0) { return newGraph; }

    for (auto& map : mapping)
    {
        FName ParamName = map.Key;
        FString OutputIdentifier = map.Value;

        for (auto& OutputPair : newGraph->OutputInstances)
        {
            USubstanceOutputData* OutputData = OutputPair.Value;
            if (!OutputData) { continue; }

            UTexture2D* OutputTexture = Cast<UTexture2D>(OutputData->GetData());
            if (!OutputTexture) { continue; }

            SubstanceAir::OutputInstance* OutputInstance = Substance::Helpers::GetSubstanceOutputByID(newGraph, OutputPair.Key);
            if (!OutputInstance) { continue; }

            FString NewOutputIdentifier = FString(OutputInstance->mDesc.mIdentifier.c_str());
            if (NewOutputIdentifier != OutputIdentifier) { continue; }

            dimMaterial->SetTextureParameterValue(ParamName, OutputTexture);

            break;
        }
    }

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

    return { { deformExist, deform }, overlays };
}

void ASymptomViewer::Reset()
{
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
            entry.SubstanceInstance->MarkAsGarbage();
            entry.SubstanceInstance = nullptr;
        }
    }

    for (auto& kv : _bodyParts)
    {
        kv.Value.OverlayEntries.Empty();
    }

    _toRender.Empty();
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
            }

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

    if (material)
    {
        newEntry.DynamicMaterial = UMaterialInstanceDynamic::Create(material, this, FName("OverlayMaterial"));
        newEntry.MeshComponent->SetMaterial(0, newEntry.DynamicMaterial);
    }

    _pool.Add(newEntry);
    return &_pool.Last();
}

//RENDERER
void ASymptomViewer::RenderTick()
{
    if (_toRender.IsEmpty())
    {
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(_renderTimerHandle);
        }

        OnRenderComplete.Broadcast();

        return;
    }

    TArray<USubstanceGraphInstance*> completed;

    for (USubstanceGraphInstance* graph : _toRender)
    {
        if (!graph || !graph->Instance) { continue; }

        bool ready = false;
        for (auto& pair : graph->OutputInstances)
        {
            if (!pair.Value) { continue; }

            UTexture2D* tex = Cast<UTexture2D>(pair.Value->GetData());
            if (tex && tex->GetPlatformData() && tex->GetPlatformData()->Mips.Num() > 4)
            {
                ready = true;
                break;
            }
        }

        if (ready) completed.Add(graph);
    }

    for (USubstanceGraphInstance* graph : completed)
    {
        for (auto& pair : graph->OutputInstances)
        {
            SubstanceAir::OutputInstance* output = Substance::Helpers::GetSubstanceOutputByID(graph, pair.Key);
            if (output)
            {
                output->flagAsDirty();
            }
        }

        TArray<SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>> singleGraph;
        singleGraph.Add(graph->Instance);
        Substance::Helpers::RenderSync(singleGraph, true);

        _toRender.Remove(graph);
    }
}