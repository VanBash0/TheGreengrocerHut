#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BookViewer.generated.h"

UCLASS()
class THEGREENGROCERHUT_API ABookViewer : public AActor
{
	GENERATED_BODY()
	
public:	
	ABookViewer();

	virtual void BeginPlay() override;
};
