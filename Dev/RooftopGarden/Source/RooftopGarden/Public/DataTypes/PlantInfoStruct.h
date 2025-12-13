// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlantInfoStruct.generated.h"

USTRUCT(BlueprintType, meta = (ToolTip = "Struct containing a plant's info for growth and harvesting purposes."))
struct FPlantInfoStruct : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 GrowthStageCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TArray<TObjectPtr<UStaticMesh>> GrowthStageMeshes = {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FName ProduceID = NAME_None;

};
