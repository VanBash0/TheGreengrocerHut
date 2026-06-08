#include "SymptomViewerLogic.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "SubstanceGraphInstance.h"
#include "SubstanceOutputData.h"
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

void USymptomViewerLogic::InitializePool(UStaticMeshComponent* rootMesh, UDataTable* symptomsTable, UDataTable* defaultBodyPartTable, UDataTable* substanceConfigTable)
{
	_rootMesh = rootMesh;
	_symptomsTable = symptomsTable;
	_defaultBodyPartTable = defaultBodyPartTable;
    _substanceConfigTable = substanceConfigTable;
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

    for (int partIndex = 1; partIndex < (int)EBodyPart::MAX; partIndex++)
    {
        EBodyPart partType = static_cast<EBodyPart>(partIndex);

        FBodyPartData* bodyData = &_bodyParts.FindOrAdd(partType);

        TArray<FSymptomRow>* partSymptoms = symptomsByPart.Find(partType);

        FName RowName = FName(StaticEnum<EBodyPart>()->GetNameStringByValue((int32)partType));
        FBodyPart* tableRow = _defaultBodyPartTable->FindRow<FBodyPart>(RowName, TEXT("GetBodyPart"));

        UStaticMesh* meshToUse = nullptr;
        UTexture2D* maskToUse = nullptr;
        TArray<FVisualOverlay> overlays;

        if (partSymptoms && partSymptoms->Num() > 0)
        {
            auto visual = SelectBodySymptomsByType(*partSymptoms);

            if (visual.first.first)
            {
                meshToUse = visual.first.second.OverrideBody.Mesh;
                maskToUse = visual.first.second.OverrideBody.RGB_Mask;
            }
            else if (tableRow)
            {
                meshToUse = tableRow->Mesh;
                maskToUse = tableRow->RGB_Mask;
            }

            overlays = visual.second;
        }
        else if (tableRow)
        {
            meshToUse = tableRow->Mesh;
            maskToUse = tableRow->RGB_Mask;
        }

        if (!bodyData->BaseMeshComp)
        {
            bodyData->BaseMeshComp = NewObject<UStaticMeshComponent>(this);
            bodyData->BaseMeshComp->SetCastShadow(false);
            bodyData->BaseMeshComp->SetupAttachment(_rootMesh);
            bodyData->BaseMeshComp->RegisterComponent();
        }

        bodyData->BaseMeshComp->SetStaticMesh(meshToUse);
        bodyData->BaseMeshComp->SetVisibility(false);

        for (const FVisualOverlay& overlay : overlays)
        {
            FVisualOverlayPoolEntry* element = GetPoolEntry(bodyData->BaseMeshComp, overlay.Material);

            if (!element || !element->MeshComponent) { continue; }
            
            element->MeshComponent->SetStaticMesh(meshToUse);
            element->MeshComponent->SetVisibility(false);

            if (element->DynamicMaterial)
            {
                element->DynamicMaterial->SetVectorParameterValue(FName(TEXT("Chanel")), TextureChannelToVector4(overlay.LayerChannel));
                element->DynamicMaterial->SetTextureParameterValue(FName(TEXT("Mask")), maskToUse);
            }

            if (!overlay.SubstanceGraph) { continue; }
            
            USubstanceGraphInstance* newGraph = overlay.SubstanceGraph->Duplicate();

            if (!newGraph) { continue; }
            
            newGraph->ConditionalPostLoad();

            newGraph->SetInputInt("$outputsize", TArray<int32>{ 10, 10 });

            newGraph->CreateOutputs();

            TArray<FString> outputNames = newGraph->GetOutputNames();
            for (const FString& outputName : outputNames)
            {
                newGraph->EnableOutput(outputName, true);
            }

            newGraph->SetInputInt("$randomseed", TArray<int32>{  FMath::RandRange(0, 100000) });
            newGraph->ApplyPreset(TEXT("DEFAULT"));
            newGraph->RenderSync();

            FSubstanceMaterialConfig* FoundConfig = nullptr;
            UMaterialInterface* OverlayParentMaterial = nullptr;

            if (overlay.Material)
            {
                UMaterialInstance* MatInstance = Cast<UMaterialInstance>(overlay.Material);
                if (MatInstance && MatInstance->Parent)
                {
                    OverlayParentMaterial = MatInstance->Parent;
                }
            }

            if (_substanceConfigTable && OverlayParentMaterial)
            {
                TArray<FName> RowNames = _substanceConfigTable->GetRowNames();

                for (const FName& ConfigRowName : RowNames)
                {
                    FSubstanceMaterialConfig* Config = _substanceConfigTable->FindRow<FSubstanceMaterialConfig>(ConfigRowName, TEXT(""));
                    if (Config && Config->ParentMaterial == OverlayParentMaterial)
                    {
                        FoundConfig = Config;
                        break;
                    }
                }
            }

            if (FoundConfig && FoundConfig->TextureMappings.Num() > 0)
            {
                for (const FSubstanceTextureMapping& Mapping : FoundConfig->TextureMappings)
                {
                    for (auto& OutputPair : newGraph->OutputInstances)
                    {
                        USubstanceOutputData* OutputData = OutputPair.Value;
                        if (OutputData)
                        {
                            UObject* DataObject = OutputData->GetData();
                            UTexture2D* OutputTexture = Cast<UTexture2D>(DataObject);

                            if (OutputTexture)
                            {
                                FString TextureName = OutputTexture->GetName().ToLower();
                                if (TextureName.Contains(Mapping.SubstanceOutputName.ToLower()))
                                {
                                    if (element->DynamicMaterial)
                                    {
                                        element->DynamicMaterial->SetTextureParameterValue(Mapping.MaterialParameterName, OutputTexture);
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            element->SubstanceInstance = newGraph;
        }
    }
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
                entry.MeshComponent->AttachToComponent(root, FAttachmentTransformRules::KeepRelativeTransform);
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
    }
    newEntry.MeshComponent->RegisterComponent();

    if (material)
    {
        newEntry.DynamicMaterial = UMaterialInstanceDynamic::Create(material, this, FName("OverlayMaterial"));
        newEntry.MeshComponent->SetMaterial(0, newEntry.DynamicMaterial);
    }

    return &_pool[_pool.Add(newEntry)];
}
