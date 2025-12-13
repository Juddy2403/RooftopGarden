#pragma once

#include "DataTypes/ItemTypeEnum.h"
#include "CoreMinimal.h"
#include "SlotStruct.generated.h"

USTRUCT(BlueprintType, meta = (ToolTip = "Struct containing an inventory slot's info: to be found in the item data table."))
struct FSlotStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* Item = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType = EItemType::Default;
};