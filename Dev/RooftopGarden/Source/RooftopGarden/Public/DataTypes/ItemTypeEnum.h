// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemTypeEnum.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Seed,
	Food,
	Tool,
	Bug,
	Weed,
	Default
};