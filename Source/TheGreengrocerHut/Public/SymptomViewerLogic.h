#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SympromsStructs.h"
#include "SymptomViewerLogic.generated.h"

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

UCLASS(BlueprintType, Blueprintable)
class THEGREENGROCERHUT_API USymptomViewerLogic : public UObject
{
	GENERATED_BODY()
public:
	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable)
	void InitializePool(UStaticMeshComponent* rootMesh, UDataTable* symptomsTable, UDataTable* defaultBodyPartTable, UDataTable* substanceConfigTable);

	UFUNCTION(BlueprintCallable)
	void SetNewSymptoms(const TArray<FName>& symptomNames);

	UFUNCTION(BlueprintCallable)
	void ShowBodyPart(const EBodyPart& partType);
private:
	void Reset();
	FVisualOverlayPoolEntry* GetPoolEntry(UStaticMeshComponent* root, UMaterialInterface* material);

	const std::pair<std::pair<bool, FVisualDeformation>, TArray<FVisualOverlay>> SelectBodySymptomsByType(const TArray<FSymptomRow>& symptoms);

	UStaticMeshComponent* _rootMesh = nullptr;
	UDataTable* _symptomsTable;
	UDataTable* _defaultBodyPartTable;
	UDataTable* _substanceConfigTable;

	TMap<EBodyPart, FBodyPartData> _bodyParts;
	TArray<FVisualOverlayPoolEntry> _pool;
};
