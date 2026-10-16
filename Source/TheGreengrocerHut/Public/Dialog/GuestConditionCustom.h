#pragma once

#include "CoreMinimal.h"
#include "DlgConditionCustom.h"
#include "GuestConditionCustom.generated.h"

UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew)
class THEGREENGROCERHUT_API UGuestConditionCustom : public UDlgConditionCustom
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialogue")
	FText GetReason(const UDlgContext* Context, const UObject* Participant, bool bMet) const;
	virtual FText GetReason_Implementation(const UDlgContext* Context, const UObject* Participant, bool bMet) const
	{
		return bMet ? MetReason : NotMetReason;
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText MetReason;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText NotMetReason;
};
