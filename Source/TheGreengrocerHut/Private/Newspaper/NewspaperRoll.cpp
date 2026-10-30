#include "Newspaper/NewspaperRoll.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"
#include "Engine/TextureRenderTarget2D.h"

ANewspaperRoll::ANewspaperRoll()
{
	PrimaryActorTick.bCanEverTick = false;

	WidgetCom = CreateDefaultSubobject<UWidgetComponent>(TEXT("Front"));
	WidgetCom->SetupAttachment(RootComponent);
	WidgetCom->SetWidgetSpace(EWidgetSpace::World);
	WidgetCom->SetRenderInMainPass(false);
	WidgetCom->SetVisibility(false);
	WidgetCom->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ANewspaperRoll::BeginPlay()
{
	if (USkeletalMeshComponent* Mesh = GetSkeletalMeshComponent())
	{
		int32 materialIndex = FMath::RandHelper(NewspaperMaterials.Num());
		MID_Newspaper = Mesh->CreateDynamicMaterialInstance(0, NewspaperMaterials[materialIndex]);
	}

	if (WidgetCom && NewspaperWidgetClass)
	{
		if (UUserWidget* RightWidget = CreateWidget<UUserWidget>(GetWorld(), NewspaperWidgetClass))
		{
			WidgetCom->SetWidget(RightWidget);
			WidgetCom->SetVisibility(true);
		}
	}

	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					if (MID_Newspaper && WidgetCom)
					{
						MID_Newspaper->SetTextureParameterValue(TEXT("FrontSide"), WidgetCom->GetRenderTarget());
					}

					WidgetCom->SetVisibility(false);
				}));
		}));
}
