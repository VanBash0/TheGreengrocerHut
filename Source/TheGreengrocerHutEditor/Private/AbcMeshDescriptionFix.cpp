#include "ContentBrowserModule.h"
#include "Engine/SkeletalMesh.h"
#include "HAL/IConsoleManager.h"
#include "IContentBrowserSingleton.h"
#include "MeshDescription.h"
#include "Modules/ModuleManager.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "Rendering/SkeletalMeshModel.h"

// Skeletal meshes imported from Alembic have no MeshDescription, which makes the
// Skeletal Mesh Editing Tools crash when the asset is opened. This builds one for LOD0
// from the existing render geometry (morph targets included) and stores it on the asset.
static void AddMeshDescriptionToSelectedSkeletalMeshes()
{
    TArray<FAssetData> SelectedAssets;
    FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
    ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

    int32 NumFixed = 0;
    for (const FAssetData& AssetData : SelectedAssets)
    {
        USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(AssetData.GetAsset());
        if (!SkeletalMesh || !SkeletalMesh->IsSourceModelValid(0) || SkeletalMesh->HasMeshDescription(0))
        {
            continue;
        }

        FSkeletalMeshModel* ImportedModel = SkeletalMesh->GetImportedModel();
        if (!ImportedModel || !ImportedModel->LODModels.IsValidIndex(0))
        {
            continue;
        }

        FMeshDescription MeshDescription;
        ImportedModel->LODModels[0].GetMeshDescription(SkeletalMesh, 0, MeshDescription);
        SkeletalMesh->CreateMeshDescription(0, MoveTemp(MeshDescription));
        SkeletalMesh->CommitMeshDescription(0);
        SkeletalMesh->PostEditChange();

        UE_LOG(LogTemp, Display, TEXT("FixAbcSkeletalMesh: added MeshDescription to %s"), *SkeletalMesh->GetPathName());
        ++NumFixed;
    }

    UE_LOG(LogTemp, Display, TEXT("FixAbcSkeletalMesh: fixed %d of %d selected asset(s). Save them to keep the change."), NumFixed, SelectedAssets.Num());
}

static FAutoConsoleCommand GFixAbcSkeletalMeshCommand(
    TEXT("FixAbcSkeletalMesh"),
    TEXT("Adds a LOD0 MeshDescription to the Skeletal Meshes selected in the Content Browser (fixes the crash on opening Alembic-imported meshes)."),
    FConsoleCommandDelegate::CreateStatic(&AddMeshDescriptionToSelectedSkeletalMeshes));
