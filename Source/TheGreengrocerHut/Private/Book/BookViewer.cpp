#include "Book/BookViewer.h"
#include "Book/Actors/Book.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ABookViewer::ABookViewer()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	CameraRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CameraRoot"));
	CameraRoot->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraRoot);

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(RootComponent);
	Light->SetMobility(EComponentMobility::Movable);
	Light->SetIntensity(0.0f);
	Light->SetVisibility(false);
}

void ABookViewer::BeginPlay()
{
	Super::BeginPlay();

	Camera->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);

	for (const FBookSlot& Slot : BookSlots)
	{
		if (!Slot.Book) { continue; }

		Slot.Book->OwnerViewer = this;
		Slot.Book->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}
}

void ABookViewer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState != EBookViewerState::Active) { return; }

	UpdateFocusFromCursor();
	UpdateCameraRig(DeltaTime);
}

bool ABookViewer::TryEnter(ABook* StartBook)
{
	if (CurrentState != EBookViewerState::Inactive || BookSlots.IsEmpty()) { return false; }

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) { return false; }

	const int32 StartIndex = FindBookSlotIndexByBookActor(StartBook);
	if (StartIndex != INDEX_NONE)
	{
		SetFocusedIndex(StartIndex);
	}
	else if (!BookSlots.IsValidIndex(FocusedIndex))
	{
		SetFocusedIndex(0);
	}

	SnapCameraToFocus();

	ActivePC = PC;
	PreviousViewTarget = PC->GetViewTarget();

	bPendingExit = false;
	bSwitchArmed = false;

	CurrentState = EBookViewerState::Entering;
	SetActorTickEnabled(true);

	PC->SetViewTargetWithBlend(this, EnterBlendTime);

	if (EnterBlendTime > KINDA_SMALL_NUMBER)
	{
		GetWorldTimerManager().SetTimer(StateTimerHandle, this, &ABookViewer::FinishEnter, EnterBlendTime, false);
	}
	else
	{
		FinishEnter();
	}

	return true;
}

void ABookViewer::FinishEnter()
{
	if (CurrentState != EBookViewerState::Entering) { return; }

	CurrentState = EBookViewerState::Active;

	if (bPendingExit)
	{
		bPendingExit = false;
		TryExit();
	}
}

bool ABookViewer::TryExit()
{
	if (CurrentState == EBookViewerState::Entering)
	{
		bPendingExit = true;
		return true;
	}

	if (CurrentState != EBookViewerState::Active) { return false; }

	CurrentState = EBookViewerState::Exiting;

	if (APlayerController* PC = ActivePC.Get())
	{
		AActor* Target = PreviousViewTarget.Get();
		if (!Target)
		{
			Target = PC->GetPawn();
		}

		PC->SetViewTargetWithBlend(Target, ExitBlendTime);
	}

	if (ExitBlendTime > KINDA_SMALL_NUMBER)
	{
		GetWorldTimerManager().SetTimer(StateTimerHandle, this, &ABookViewer::FinishExit, ExitBlendTime, false);
	}
	else
	{
		FinishExit();
	}

	return true;
}

void ABookViewer::FinishExit()
{
	if (CurrentState != EBookViewerState::Exiting) { return; }

	CurrentState = EBookViewerState::Inactive;

	ActivePC.Reset();
	PreviousViewTarget.Reset();

	SetActorTickEnabled(false);
}

void ABookViewer::SnapCameraToFocus()
{
	if (!BookSlots.IsValidIndex(FocusedIndex)) { return; }

	const FTransform& Pose = BookSlots[FocusedIndex].ViewTransfrom;

	RigLocation = Pose.GetLocation();
	RigRotation = Pose.GetRotation().GetNormalized();

	CameraRoot->SetRelativeLocationAndRotation(RigLocation, RigRotation);
}

void ABookViewer::SetFocusedIndex(int32 NewIndex)
{
	if (!BookSlots.IsValidIndex(NewIndex) || NewIndex == FocusedIndex) { return; }

	ABook* Previous = GetFocusedBook();
	FocusedIndex = NewIndex;

	OnFocusedBookChanged.Broadcast(Previous, GetFocusedBook(), FocusedIndex);
}

int32 ABookViewer::FindBookSlotIndexByBookActor(const ABook* Book) const
{
	if (!Book) { return INDEX_NONE; }

	return BookSlots.IndexOfByPredicate([Book](const FBookSlot& Slot) { return Slot.Book.Get() == Book; });
}

ABook* ABookViewer::GetFocusedBook() const
{
	return BookSlots.IsValidIndex(FocusedIndex) ? BookSlots[FocusedIndex].Book.Get() : nullptr;
}

void ABookViewer::UpdateFocusFromCursor()
{
	APlayerController* PC = ActivePC.Get();
	if (!PC) { return; }

	float MouseX = 0.0f;
	float MouseY = 0.0f;
	if (!PC->GetMousePosition(MouseX, MouseY)) { return; }

	const FVector2D Cursor(MouseX, MouseY);

	int32 NearestIndex = INDEX_NONE;
	float NearestDistance = TNumericLimits<float>::Max();
	float FocusedDistance = TNumericLimits<float>::Max();

	for (int32 i = 0; i < BookSlots.Num(); ++i)
	{
		FBox2D Rect(ForceInit);
		if (!GetBookScreenRect(BookSlots[i].Book, PC, Rect)) { continue; }

		const float Distance = DistanceToRect(Rect, Cursor);

		if (i == FocusedIndex)
		{
			FocusedDistance = Distance;
		}

		if (Distance < NearestDistance)
		{
			NearestDistance = Distance;
			NearestIndex = i;
		}
	}

	if (FocusedDistance == TNumericLimits<float>::Max()) { return; }

	if (FocusedDistance <= 0.0f)
	{
		bSwitchArmed = true;
	}

	if (bRequireHoverBeforeSwitch && !bSwitchArmed) { return; }

	if (NearestIndex != INDEX_NONE
		&& NearestIndex != FocusedIndex
		&& NearestDistance + SwitchHysteresis < FocusedDistance)
	{
		SetFocusedIndex(NearestIndex);
	}
}

void ABookViewer::UpdateCameraRig(float DeltaTime)
{
	if (!BookSlots.IsValidIndex(FocusedIndex)) { return; }

	const FTransform& Pose = BookSlots[FocusedIndex].ViewTransfrom;

	RigLocation = FMath::VInterpTo(RigLocation, Pose.GetLocation(), DeltaTime, CameraSwitchSpeed);
	RigRotation = FMath::QInterpTo(RigRotation, Pose.GetRotation().GetNormalized(), DeltaTime, CameraSwitchSpeed);

	CameraRoot->SetRelativeLocationAndRotation(RigLocation, RigRotation);
}

bool ABookViewer::GetBookScreenRect(const ABook* Book, APlayerController* PC, FBox2D& OutRect) const
{
	const UBoxComponent* Box = Book ? Book->BookCollision.Get() : nullptr;
	if (!Box) { return false; }

	const FTransform BoxTransform = Box->GetComponentTransform();
	const FVector Extent = Box->GetUnscaledBoxExtent();

	OutRect = FBox2D(ForceInit);

	int32 Projected = 0;
	for (int32 i = 0; i < 8; ++i)
	{
		const FVector LocalCorner(
			(i & 1) ? Extent.X : -Extent.X,
			(i & 2) ? Extent.Y : -Extent.Y,
			(i & 4) ? Extent.Z : -Extent.Z);

		FVector2D Screen;
		if (UGameplayStatics::ProjectWorldToScreen(PC, BoxTransform.TransformPosition(LocalCorner), Screen, false))
		{
			OutRect += Screen;
			++Projected;
		}
	}

	return Projected == 8;
}

float ABookViewer::DistanceToRect(const FBox2D& Rect, const FVector2D& Point)
{
	const double DX = FMath::Max3<double>(Rect.Min.X - Point.X, 0.0, Point.X - Rect.Max.X);
	const double DY = FMath::Max3<double>(Rect.Min.Y - Point.Y, 0.0, Point.Y - Rect.Max.Y);

	return static_cast<float>(FMath::Sqrt(DX * DX + DY * DY));
}