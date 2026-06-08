#include "SymptomViewerLogic.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "SubstanceGraphInstance.h"
#include "SubstanceOutputData.h"
#include "SubstanceCoreHelpers.h"
#include "SubstanceConfigLibrary.h"

void USymptomViewerLogic::BeginDestroy()
{
	for (FVisualOverlayPoolEntry& entry : _pool)
	{
		if (entry.SubstanceInstance)
		{
			entry.SubstanceInstance->ConditionalBeginDestroy();
			entry.SubstanceInstance = nullptr;
		}

        if (entry.MeshComponent)
        {
            entry.MeshComponent->ConditionalBeginDestroy();
            entry.MeshComponent = nullptr;
        }
	}

	for (auto& kv : _bodyParts)
	{
		if (kv.Value.BaseMeshComp)
		{
			kv.Value.BaseMeshComp->ConditionalBeginDestroy();
			kv.Value.BaseMeshComp = nullptr;
		}
	}

	_bodyParts.Empty();
	_pool.Empty();

	Super::BeginDestroy();
}

void USymptomViewerLogic::InitializePool(UStaticMeshComponent* rootMesh, UDataTable* symptomsTable, UDataTable* defaultBodyPartTable)
{
	_rootMesh = rootMesh;

	_symptomsTable = symptomsTable;
    _defaultBodyPartTable = defaultBodyPartTable;

    for (int partIndex = 1; partIndex < (int)EBodyPart::MAX; partIndex++)
    {
        EBodyPart partType = static_cast<EBodyPart>(partIndex);

        FBodyPartData bodyData = {};
        bodyData.BaseMeshComp = NewObject<UStaticMeshComponent>(this);
        bodyData.BaseMeshComp->SetCastShadow(false);
        bodyData.BaseMeshComp->SetupAttachment(_rootMesh);
        bodyData.BaseMeshComp->RegisterComponent();
        bodyData.BaseMeshComp->SetVisibility(false);
        
        _bodyParts.Add(partType, bodyData);
    }
}

void USymptomViewerLogic::SetNewSymptoms(const TArray<FName>& symptomNames)
{
    Reset();

    if (!_symptomsTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Symptoms table not assigned!"));
        return;
    }

    if (!_defaultBodyPartTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Body parts table not assigned!"));
        return;
    }

    FString ContextString = TEXT("Getting Symptom Data");
    TMap<EBodyPart, TArray<FSymptomRow>> symptomsByPart;

    for (const FName& name : symptomNames)
    {
        if (FSymptomRow* symptom = _symptomsTable->FindRow<FSymptomRow>(name, ContextString))
        {
            symptomsByPart.FindOrAdd(symptom->Type).Add(*symptom);
        }
    }

    TArray<SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>> graphToRender;

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
            FBodyPart* tableRow = _defaultBodyPartTable->FindRow<FBodyPart>(RowName, TEXT("GetBodyPart"));
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
                element->DynamicMaterial->SetVectorParameterValue(FName(TEXT("Chanel")), TextureChannelToVector4(overlay.LayerChannel));
                element->DynamicMaterial->SetTextureParameterValue(FName(TEXT("Mask")), maskToUse);
            }

            bodyData->OverlayEntries.Add(element);

            if (!overlay.SubstanceGraph) { continue; }

            USubstanceGraphInstance* newGraph = CopyGraphAndSetMaterial(overlay.SubstanceGraph, overlay.Material, element->DynamicMaterial);

            if (newGraph && newGraph->Instance)
            {
                graphToRender.Add(newGraph->Instance);
                element->SubstanceInstance = newGraph;
            }
        }

        bodyData->Hide();
    }

    Substance::Helpers::RenderSync(graphToRender, true);
}

USubstanceGraphInstance* USymptomViewerLogic::CopyGraphAndSetMaterial(USubstanceGraphInstance* graph, UMaterialInterface* mainMaterial, UMaterialInstanceDynamic* dimMaterial)
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

void USymptomViewerLogic::ShowBodyPart(const EBodyPart& partType)
{
    for (auto& kv : _bodyParts)
    {
        if (kv.Key == partType)
        {
            kv.Value.Show();
        }
        else
        {
            kv.Value.Hide();
        }
    }
}

const std::pair<std::pair<bool, FVisualDeformation>, TArray<FVisualOverlay>> USymptomViewerLogic::SelectBodySymptomsByType(const TArray<FSymptomRow>& symptoms)
{
	bool deformExist = false;
	FVisualDeformation deform;

	TArray<FVisualOverlay> overlays;

	for (const FSymptomRow& s : symptoms)
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

void USymptomViewerLogic::Reset()
{
	for (FVisualOverlayPoolEntry& entry : _pool)
	{
		entry.bIsInUse = false;

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
}

FVisualOverlayPoolEntry* USymptomViewerLogic::GetPoolEntry(UStaticMeshComponent* root, UMaterialInterface* material)
{
    for (FVisualOverlayPoolEntry& entry : _pool)
    {
        if (!entry.bIsInUse)
        {
            entry.bIsInUse = true;

            if (material && entry.MeshComponent)
            {
                if (entry.DynamicMaterial)
                {
                    entry.DynamicMaterial->MarkAsGarbage();
                }

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

    return &_pool[_pool.Add(newEntry)];
}