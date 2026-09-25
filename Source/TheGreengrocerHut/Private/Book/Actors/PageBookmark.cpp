#include "Book/Actors/PageBookmark.h"
#include "Book/Actors/Book.h"
#include "Book/Actors/Page.h"
#include "Book/Data/BookData.h"
#include "Book/Data/CoverData.h"
#include "Book/Data/PageData.h"
#include "Book/Widget/BookPageBase.h"

APageBookmark::APageBookmark()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APageBookmark::BeginPlay()
{
	Super::BeginPlay();

	if (USkeletalMeshComponent* Mesh = GetSkeletalMeshComponent())
	{
		Mesh->OnBeginCursorOver.AddDynamic(this, &APageBookmark::HandleBeginCursorOver);
		Mesh->OnEndCursorOver.AddDynamic(this, &APageBookmark::HandleEndCursorOver);
		Mesh->OnClicked.AddDynamic(this, &APageBookmark::HandleCursorClicked);
	}
}

void APageBookmark::InitializeBookmark(ABook* InOwningBook, TSubclassOf<UBookPageBase> InChapterType)
{
	OwningBook = InOwningBook;
	ChapterType = InChapterType;
	bIsFixedBookmark = false;

	if (!OwningBook || !OwningBook->BookData) { return; }

	if (FChapterRuntimeInfo* RInfo = OwningBook->ChapterMetrics.Find(ChapterType))
	{
		TargetPage = RInfo->StartIndex;
	}

	if (USkeletalMeshComponent* Mesh = GetSkeletalMeshComponent())
	{
		for (const auto& Data : OwningBook->BookData->PagesInfo)
		{
			if (Data.PageType == ChapterType)
			{
				Mesh->SetMaterial(0, Data.BookmarkMaterial);
				break;
			}
		}
	}

	AttachToBook();
}

void APageBookmark::InitializeFixedBookmark(ABook* InOwningBook, int32 InTargetPage, float InYOffset)
{
	OwningBook = InOwningBook;
	bIsFixedBookmark = true;
	TargetPage = InTargetPage;
	FixedYOffset = InYOffset;

	if (!OwningBook) { return; }

	AttachToBook();
}

void APageBookmark::AttachToBook()
{
	if (!OwningBook || !OwningBook->BookData || !OwningBook->BookData->Cover) { return; }

	UCoverData* Cover = OwningBook->BookData->Cover;
	FName SoketName = bOnRightSide ? Cover->BoneToAttachBookmark_Side_R : Cover->BoneToAttachBookmark_Side_L;
	AttachToComponent(OwningBook->MeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale, SoketName);

	if (bIsFixedBookmark)
	{
		SetActorRelativeLocation(OwningBook->GetBookmarkLocationForPage(TargetPage, FixedYOffset));
		OwningBook->ExitBookmark = this;
	}
	else
	{
		SetActorRelativeLocation(OwningBook->GetBookmarkAttachedLocation(ChapterType));
		OwningBook->Bookmarks.Add(ChapterType, this);
	}
}

void APageBookmark::AttachToPage(APage* PageToAttaching)
{
	AttachedPage = PageToAttaching;

	FName Socket = AttachedPage->PageData->BoneToAttachBookmark;
	AttachToComponent(AttachedPage->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale, Socket);

	const float YOffset = bIsFixedBookmark ? FixedYOffset : OwningBook->GetBookmarkAttachedLocation(ChapterType).Y;
	SetActorRelativeLocation(FVector(0.0f, YOffset, 0.0f));

	AttachedPage->OnRelease.AddDynamic(this, &APageBookmark::OnAttachedPageReleased);
	AttachedPage->OnClick.AddDynamic(this, &APageBookmark::OnAttachedPageClicked);

	if (bIsFixedBookmark)
	{
		if (OwningBook->ExitBookmark == this)
		{
			OwningBook->ExitBookmark = nullptr;
		}
	}
	else
	{
		OwningBook->Bookmarks.Remove(ChapterType);
	}
}

void APageBookmark::OnAttachedPageReleased()
{
	AttachedPage->OnRelease.RemoveDynamic(this, &APageBookmark::OnAttachedPageReleased);
	AttachedPage->OnClick.RemoveDynamic(this, &APageBookmark::OnAttachedPageClicked);

	AttachedPage = nullptr;

	AttachToBook();
}

void APageBookmark::OnAttachedPageClicked()
{
	bOnRightSide = !bOnRightSide;
}

bool APageBookmark::CanBookmarkHovering_Implementation()
{
	return OwningBook->CurPageNumber != TargetPage;
}

void APageBookmark::HandleBeginCursorOver(UPrimitiveComponent* TouchedComponent)
{
	if (bIsBookmarkHovering) { return; }

	if (!OwningBook) { return; }

	if (!CanBookmarkHovering()) { return; }

	bIsBookmarkHovering = true;

	OnHovering.Broadcast(bIsBookmarkHovering);
}

void APageBookmark::HandleEndCursorOver(UPrimitiveComponent* TouchedComponent)
{
	if (!bIsBookmarkHovering) { return; }

	bIsBookmarkHovering = false;

	OnHovering.Broadcast(bIsBookmarkHovering);
}

void APageBookmark::HandleCursorClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	if (!bIsBookmarkHovering) { return; }

	OnBookmarkClicked();
}

void APageBookmark::OnBookmarkClicked_Implementation()
{
	if (!OwningBook) { return; }

	OwningBook->GoToPage(TargetPage);
}