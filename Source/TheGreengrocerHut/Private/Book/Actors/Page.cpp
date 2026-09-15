#include "Book/Actors/Page.h"
#include "Book/Actors/Book.h"
#include "Book/Widget/BookPageBase.h"
#include "Book/Data/PageData.h"

#include "Engine/TextureRenderTarget2D.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"

#include "Materials/MaterialInstanceDynamic.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

APage::APage()
{
	PrimaryActorTick.bCanEverTick = false;

	Front = CreateDefaultSubobject<UWidgetComponent>(TEXT("Front"));
	Front->SetupAttachment(RootComponent);
	Front->SetWidgetSpace(EWidgetSpace::World);
	Front->SetRenderInMainPass(false);
	Front->SetVisibility(false);
	Front->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Back = CreateDefaultSubobject<UWidgetComponent>(TEXT("Back"));
	Back->SetupAttachment(RootComponent);
	Back->SetWidgetSpace(EWidgetSpace::World);
	Back->SetRenderInMainPass(false);
	Back->SetVisibility(false);
	Back->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APage::OnFinish()
{
	bOnRightSide = !bOnRightSide;
	bIsFlippingProcess = false;

	CheckHovering();

	OnFinishFlip.Broadcast();
}

void APage::BeginPlay()
{
	Super::BeginPlay();

	if (USkeletalMeshComponent* Mesh = GetSkeletalMeshComponent())
	{
		Mesh->OnClicked.AddDynamic(this, &APage::HandleClicked);
		Mesh->OnBeginCursorOver.AddDynamic(this, &APage::HandleBeginCursorOver);
		Mesh->OnEndCursorOver.AddDynamic(this, &APage::HandleEndCursorOver);
	}
}

void APage::InitializePage(ABook* InOwnerBook, UPageData* InPageData, int32 InPageNumber, bool bInIsRightSide)
{
	OwnerBook = InOwnerBook;
	PageData = InPageData;
	PageNumber = InPageNumber;
	bOnRightSide = bInIsRightSide;

	bIsFlippingProcess = false;
	bIsHover = false;

	if (USkeletalMeshComponent* Mesh = GetSkeletalMeshComponent())
	{
		if (Mesh->GetAnimInstance())
		{
			Mesh->GetAnimInstance()->StopAllMontages(0.0f);
		}

		if (PageData)
		{
			if (PageData->SM_Page)
			{
				Mesh->SetSkeletalMesh(PageData->SM_Page);
			}
			
			int32 materialIndex = PageData->RandomSeedForSelectMaterial % 3;
			materialIndex *= PageNumber / 2;
			materialIndex += PageData->RandomSeedForSelectMaterial / 11;
			materialIndex %= PageData->MaterialVariantion.Num();

			MID_Page = Mesh->CreateDynamicMaterialInstance(0, PageData->MaterialVariantion[materialIndex]);
		}
	}
}

void APage::InitializeWidgets(int32 InPageIndex, TSubclassOf<UBookPageBase> InWidgetPageR, TSubclassOf<UBookPageBase> InWidgetPageL)
{
	PageIndex = InPageIndex;

	if (Front && InWidgetPageR)
	{
		if (UBookPageBase* RightWidget = CreateWidget<UBookPageBase>(GetWorld(), InWidgetPageR))
		{
			RightWidget->Initialize(PageIndex);

			Front->SetWidget(RightWidget);

			Front->SetVisibility(true);
		}
	}

	if (Back && InWidgetPageL)
	{
		if (UBookPageBase* LeftWidget = CreateWidget<UBookPageBase>(GetWorld(), InWidgetPageL))
		{
			LeftWidget->Initialize(PageIndex + 1);

			Back->SetWidget(LeftWidget);

			Back->SetVisibility(true);
		}
	}

	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (MID_Page)
			{
				if (Front)
				{
					MID_Page->SetTextureParameterValue(TEXT("FrontSide"), Front->GetRenderTarget());
				}
				
				if (Back)
				{
					MID_Page->SetTextureParameterValue(TEXT("BackSide"), Back->GetRenderTarget());
				}
			}

			Front->SetVisibility(false);
			Back->SetVisibility(false);
		}));
	}));
}

void APage::ReleasePage()
{
	PageData = nullptr;
	MID_Page = nullptr;

	PageNumber = INDEX_NONE;
	PageIndex = INDEX_NONE;
	bOnRightSide = false;
	bIsHover = false;
	bIsFlippingProcess = false;

	InitializeWidgets(0, nullptr, nullptr);
	HidePage();

	if (OwnerBook)
	{
		OwnerBook->ShowedPages.Remove(this);
		OwnerBook->PagePool.Add(this);

		OwnerBook = nullptr;
	}

	OnRelease.Broadcast();
}

void APage::PageActionOnClick()
{
	if (OwnerBook && !OwnerBook->bIsFlippingSequenceActive)
	{
		if (bOnRightSide)
		{
			OwnerBook->NextPage();
		}
		else
		{
			OwnerBook->PreviousPage();
		}

		OwnerBook->UpdateWindow();
	}
}

void APage::ShowPage()
{
	SetActorHiddenInGame(false);
}

void APage::HidePage()
{
	SetActorHiddenInGame(true);
	SetPageActive(false);
}

void APage::SetPageActive(bool bActiveClicked)
{
	if (USkeletalMeshComponent* Mesh = GetSkeletalMeshComponent())
	{
		Mesh->SetCollisionEnabled(bActiveClicked ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}
}

void APage::FlipPage()
{
	bIsFlippingProcess = true;

	OnClick.Broadcast();
}

void APage::HoverPage()
{
	bIsHover = true;
	OnHovering.Broadcast(true);
}

void APage::UnHoverPage()
{
	bIsHover = false;
	OnHovering.Broadcast(false);
}

void APage::CheckHovering()
{
	ETraceTypeQuery VisibilityTrace = UEngineTypes::ConvertToTraceType(ECC_Visibility);

	FHitResult hitResult;
	if (GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursorByChannel(VisibilityTrace, false, hitResult))
	{
		if (hitResult.Component == GetSkeletalMeshComponent())
		{
			HoverPage();
		}
		else
		{
			bIsHover = false;
		}
	}
}

void APage::HandleClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	if (!bIsFlippingProcess && bIsHover)
	{
		FlipPage();
	}
}

void APage::HandleBeginCursorOver(UPrimitiveComponent* TouchedComponent)
{
	if (!bIsFlippingProcess && !bIsHover)
	{
		HoverPage();
	}
}

void APage::HandleEndCursorOver(UPrimitiveComponent* TouchedComponent)
{
	if (!bIsFlippingProcess && bIsHover)
	{
		UnHoverPage();
	}
}