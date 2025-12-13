// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FarmingPlotDataStruct.generated.h"

USTRUCT(BlueprintType)
struct FFarmingPlotDataStruct
{
	GENERATED_BODY()
	FFarmingPlotDataStruct() = default;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	bool HasPlant = false;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	bool HasDeadPlant = false;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FName CurrentProduceID = NAME_None;
};