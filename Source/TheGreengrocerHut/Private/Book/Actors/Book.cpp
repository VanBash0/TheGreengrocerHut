#include "Book/Actors/Book.h"
#include "Book/Actors/Page.h"
#include "Book/Widget/BookPageBase.h"
#include "Book/Data/BookData.h"

ABook::ABook()
{
	PrimaryActorTick.bCanEverTick = false;

	PageRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PageRoot"));
	PageRoot->SetupAttachment(RootComponent);
}

void ABook::BeginPlay()
{
	Super::BeginPlay();

	InitializeBook();

	if (USkeletalMeshComponent* Mesh = GetSkeletalMeshComponent())
	{
		Mesh->OnClicked.AddDynamic(this, &ABook::OnBookCliked);
	}
}

void ABook::OnBookCliked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	if (!bCanFlipPage) { return; }

	ETraceTypeQuery VisibilityTrace = UEngineTypes::ConvertToTraceType(ECC_Visibility);

	FHitResult hitResult;
	if (GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursorByChannel(VisibilityTrace, false, hitResult))
	{
		if (hitResult.Component == GetSkeletalMeshComponent())
		{
			if (hitResult.BoneName == BookData->Cover->BoneToDetect_Side_R)
			{
				if (CurPageNumber == -1)
				{
					NextPage();
				}
				else if (CurPageNumber == 0)
				{
					PreviousPage();
				}
				else
				{
					return;
				}
			}
			else if (hitResult.BoneName == BookData->Cover->BoneToDetect_Side_L)
			{
				if (CurPageNumber == TotalPageCount)
				{
					NextPage();
				}
				else if (CurPageNumber > TotalPageCount)
				{
					PreviousPage();
				}
				else
				{
					return;
				}
			}

			UpdatePageRoot();
		}
	}
}

void ABook::InitializeBook()
{
	if (BookData)
	{
		if (USkeletalMeshComponent* Mesh = GetSkeletalMeshComponent())
		{
			Mesh->SetSkeletalMesh(BookData->Cover->SM_Cover);
		}
	}

	CurrentWindowSize = DefaultWindowSize;
}

void ABook::OnOpenBook_Implementation()
{
	CurrentWindowSize = DefaultWindowSize;

	UpdateWindow();

	StartOffsetPageProcess();
}

void ABook::OnCloseBook_Implementation()
{
	StopOffsetPageProcess();

	CurrentWindowSize = 2;

	UpdateWindow();
}

int32 ABook::GetNextPageNumber(int32 From) const
{
	if (From == -1) { return 0; }
	if (From == TotalPageCount) { return TotalPageCount + 1; }

	return From + 2;
}

int32 ABook::GetPreviousPageNumber(int32 From) const
{
	if (From > TotalPageCount) { return TotalPageCount; }
	if (From == 0) { return -1; }

	return From - 2;
}

void ABook::SetCurrentPage(int32 NewPage)
{
	PrevPageNumber = CurPageNumber;
	CurPageNumber = NewPage;

	OnPageChanged.Broadcast(PrevPageNumber, CurPageNumber);

	const bool bCoverTransition = (PrevPageNumber < 0 || PrevPageNumber > TotalPageCount
		|| CurPageNumber < 0 || CurPageNumber > TotalPageCount);

	if (!bCoverTransition)
	{
		UpdateWindow();
	}
}

void ABook::NextPage()
{
	if (!bCanFlipPage) { return; }
	if (CurPageNumber > TotalPageCount) { return; }

	SetCurrentPage(GetNextPageNumber(CurPageNumber));
}

void ABook::PreviousPage()
{
	if (!bCanFlipPage) { return; }
	if (CurPageNumber < 0) { return; }

	SetCurrentPage(GetPreviousPageNumber(CurPageNumber));
}

void ABook::GoToPage(int32 TargetPage)
{
	if (!bCanFlipPage) { return; }
	if (bIsFlippingSequenceActive) { return; }

	if (CurPageNumber < 0 || CurPageNumber > TotalPageCount) { return; }

	if (TargetPage == CurPageNumber) { return; }
	if (TargetPage < 0 || TargetPage > TotalPageCount) { return; }

	FinalTargetPage = TargetPage;
	SequenceStartPage = CurPageNumber;
	TargetAnimationPagesTotal = FMath::Abs(TargetPage - CurPageNumber) / 2;

	PreSequenceWindowSize = CurrentWindowSize;
	CurrentWindowSize = FMath::Max(CurrentWindowSize, FlippingWindowSize);

	bIsFlippingSequenceActive = true;

	FlipToTargetPageProcess();
}

void ABook::PageFlipProgress(float& Progress)
{
	Progress = CurPageNumber / (float)(TotalPageCount - 2);
}

UPageData* ABook::GetPageInitializeData(int32 PageN)
{
	int32 seed = BookData->RandomSeedForPageTypeSelector;
	int32 dataIndex = PageN / 2;
	dataIndex += seed / 17;
	dataIndex *= seed % 8;
	dataIndex %= BookData->Pages.Num();

	return BookData->Pages[dataIndex];
}

void ABook::GetPageWidgetData(int32 PageN, int32& PageIndex, TSubclassOf<UBookPageBase>& Widget_R, TSubclassOf<UBookPageBase>& Widget_L)
{
	PageIndex = PageN;
	Widget_R = Widget;
	Widget_L = Widget;
}

bool ABook::ShouldFullyInitializePage(int32 PageN) const
{
	if (!bIsFlippingSequenceActive) { return true; }

	const int32 K = FMath::Max(CurrentWindowSize / 2, FlippingShowInitializedPages);

	const int32 StepsFromStart = FMath::Abs(PageN - SequenceStartPage) / 2;
	const int32 StepsToTarget = FMath::Abs(FinalTargetPage - PageN) / 2;

	return StepsFromStart < K || StepsToTarget <= K;
}

bool ABook::IsPageNumberShowed(int32 PageN)
{
	for (const auto& page : ShowedPages)
	{
		if (page->PageNumber == PageN)
		{
			return true;
		}
	}

	return false;
}

APage* ABook::GetPageByNumber(int32 PageN)
{
	for (const auto& page : ShowedPages)
	{
		if (page->PageNumber == PageN)
		{
			return page;
		}
	}

	return nullptr;
}

void ABook::GetOrCreatePage(int32 PageNumber, APage*& Page, bool bFullInit)
{
	if (CurPageNumber < 0 || CurPageNumber > TotalPageCount) { return; }

	if (!TryGetFromPool(PageNumber, Page))
	{
		CreateNewPage(PageNumber, Page);
	}

	InitializePage(Page, PageNumber, bFullInit);
}

bool ABook::TryGetFromPool(int32 PageN, APage*& Page)
{
	Page = nullptr;

	if (PagePool.Num() <= 0) { return false; }

	Page = PagePool.Last();
	PagePool.Remove(Page);

	return true;
}

void ABook::CreateNewPage(int32 PageN, APage*& Page)
{
	Page = GetWorld()->SpawnActor<APage>(PageClass ? *PageClass : APage::StaticClass());
	Page->AttachToComponent(PageRoot, FAttachmentTransformRules::SnapToTargetIncludingScale);
}

void ABook::InitializePage(APage*& Page, int32 PageNumber, bool bFullInit)
{
	UPageData* data = GetPageInitializeData(PageNumber);

	bool rightSideSign = PageNumber - CurPageNumber >= 0;

	Page->InitializePage(this, data, PageNumber, rightSideSign);

	if (bFullInit)
	{
		int32 pageIndex;
		TSubclassOf<UBookPageBase> widgetR;
		TSubclassOf<UBookPageBase> widgetL;
		GetPageWidgetData(PageNumber, pageIndex, widgetR, widgetL);

		Page->InitializeWidgets(pageIndex, widgetR, widgetL);
	}

	Page->ShowPage();
	ShowedPages.Add(Page);

	Page->SetActorRelativeLocation(GetPageOffset(PageNumber));
}

void ABook::ReleaseAllPage()
{
	TArray<TObjectPtr<APage>> TempToRelease = ShowedPages;

	for (const auto& page : TempToRelease)
	{
		page->ReleasePage();
	}
}

void ABook::UpdateWindow()
{
	if (CurPageNumber < 0 || CurPageNumber > TotalPageCount) { return; }

	int32 WindowIntervalMin = FMath::Clamp(CurPageNumber - CurrentWindowSize, 0, TotalPageCount - 2);
	int32 WindowIntervalMax = FMath::Clamp(CurPageNumber + CurrentWindowSize - 2, 0, TotalPageCount - 2);

	ReleaseOutOfWindowPages(WindowIntervalMin, WindowIntervalMax);

	CreateMissingWindowPages(WindowIntervalMin, WindowIntervalMax);

	UpdatePagesActivation();
}

void ABook::ReleaseOutOfWindowPages(int32 WindowMin, int32 WindowMax, bool IgnoreAnimation)
{
	TArray<TObjectPtr<APage>> TempToRelease = { ShowedPages };

	for (const auto& page : TempToRelease)
	{
		int32 pageN = page->PageNumber;

		if (pageN < WindowMin || pageN > WindowMax)
		{
			if (!IgnoreAnimation && page->bIsFlippingProcess) { continue; }

			page->ReleasePage();
		}
	}
}

void ABook::CreateMissingWindowPages(int32 WindowMin, int32 WindowMax)
{
	int32 count = (WindowMax - WindowMin) / 2;

	for (int32 i = 0; i <= count; i++)
	{
		int32 pageN = i * 2 + WindowMin;

		if (!IsPageNumberShowed(pageN))
		{
			APage* page = nullptr;
			GetOrCreatePage(pageN, page, ShouldFullyInitializePage(pageN));
		}
	}
}

void ABook::UpdatePagesActivation()
{
	for (const auto& page : ShowedPages)
	{
		int32 delata = page->PageNumber - CurPageNumber;

		page->SetPageActive(delata == 0 || delata == -2);
	}
}

FVector ABook::GetPageOffset(int32 PageN)
{
	int32 dir = PageN - CurPageNumber;

	if (dir >= 0)
	{
		return PageOffsetDirRight * (dir / 2);
	}
	else
	{
		return PageOffsetDirLeft * (((-dir) - 2) / 2);
	}
}

void ABook::UpdatePageRoot()
{
	bool sign = CurPageNumber < 0 || CurPageNumber > TotalPageCount;

	PageRoot->SetRelativeLocation(FVector::UpVector * (sign ? -1.0f : 3.0f));
}

void ABook::StartOffsetPageProcess()
{
	GetWorldTimerManager().SetTimer(OffsetTimerHandler, this, &ABook::UpdateOffsetPageProcess, 0.05f, true);
}

void ABook::UpdateOffsetPageProcess()
{
	for (const auto& page : ShowedPages)
	{
		FVector A = page->GetRootComponent()->GetRelativeLocation();
		FVector B = GetPageOffset(page->PageNumber);

		float speed = page->PageNumber == PrevPageNumber - 2 || page->PageNumber == PrevPageNumber ? 1.5f : 7.5f;

		page->SetActorRelativeLocation(FMath::VInterpTo(A, B, 0.05f, speed));
	}
}

void ABook::StopOffsetPageProcess()
{
	if (!OffsetTimerHandler.IsValid()) { return; }

	GetWorldTimerManager().ClearTimer(OffsetTimerHandler);

	for (const auto& page : ShowedPages)
	{
		page->SetActorRelativeLocation(GetPageOffset(page->PageNumber));
	}
}

void ABook::FlipToTargetPageProcess()
{
	if (CurPageNumber == FinalTargetPage)
	{
		TryFinishFlipSequence();
		return;
	}

	const bool bGoingForward = FinalTargetPage > CurPageNumber;

	const int32 OutgoingPageNumber = bGoingForward ? CurPageNumber : CurPageNumber - 2;

	APage* OutgoingPage = GetPageByNumber(OutgoingPageNumber);
	if (OutgoingPage == nullptr)
	{
		GetOrCreatePage(OutgoingPageNumber, OutgoingPage, ShouldFullyInitializePage(OutgoingPageNumber));
	}

	if (OutgoingPage == nullptr)
	{
		TryFinishFlipSequence();
		return;
	}

	if (!OutgoingPage->FlipPage())
	{
		TryFinishFlipSequence();
		return;
	}

	if (CurPageNumber != FinalTargetPage)
	{
		GetWorldTimerManager().SetTimer(FlipOverlapTimerHandle, this, &ABook::FlipToTargetPageProcess, FlipOverlapDelay, false);
	}
	else
	{
		TryFinishFlipSequence();
	}
}

void ABook::TryFinishFlipSequence()
{
	for (const auto& page : ShowedPages)
	{
		if (page->bIsFlippingProcess)
		{
			GetWorldTimerManager().SetTimer(FlipSettleTimerHandle, this, &ABook::TryFinishFlipSequence, 0.05f, false);
			return;
		}
	}

	CurrentWindowSize = PreSequenceWindowSize;
	UpdateWindow();

	bIsFlippingSequenceActive = false;
}