// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/DataTable.h"
#include "DataTablePresets.generated.h"

/**
 * 
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Data Table Presets"))
class ROOFTOPGARDEN_API UDataTablePresets : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category="Data Tables")
	UDataTable* GetItemDataTable() const
	{
		return ItemDataTable.LoadSynchronous();
	}
	UFUNCTION(BlueprintPure, Category="Data Tables")
	UDataTable* GetPlantDataTable() const
	{
		return PlantDataTable.LoadSynchronous();
	}
private:
	UPROPERTY(EditAnywhere, Config)
	TSoftObjectPtr<UDataTable> ItemDataTable;

	UPROPERTY(EditAnywhere, Config)
	TSoftObjectPtr<UDataTable> PlantDataTable;
};
