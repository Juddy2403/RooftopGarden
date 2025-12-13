// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameTimeSettings.generated.h"

/**
 * 
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Game Time Settings"))
class ROOFTOPGARDEN_API UGameTimeSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, meta=(ToolTip="Multiplier for how fast time passes.", ClampMin="0.001", ClampMax="1"))
	float TimeSpeed = 0.5f;

	UPROPERTY(Config, EditAnywhere, meta=(ToolTip="Time of day when a new day starts. Value between 0 and 24.", ClampMin="0", ClampMax="24"))
	float DayStart = 6.f;

	UPROPERTY(Config, EditAnywhere, meta=(ToolTip="Interval in seconds at which the game time updates.", ClampMin="0.001", ClampMax="1"))
	float TickInterval = 0.05f;
};
