// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PlotPloughComponent.h"

#include "InstancedStruct.h"
#include "DataTypes/SaveDataStructs/VariantDataStruct.h"

UPlotPloughComponent::UPlotPloughComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlotPloughComponent::PloughSoil()
{
	if (bHasBeenPloughed) return;
	bHasBeenPloughed = true;
	OnPloughed.Broadcast();
}

void UPlotPloughComponent::ResetSoil()
{
	if (!bHasBeenPloughed) return;
	bHasBeenPloughed = false;
	OnPloughReset.Broadcast();
}

bool UPlotPloughComponent::HasBeenPloughed() const
{
	return bHasBeenPloughed;
}

void UPlotPloughComponent::LoadComponentState_Implementation(const TArray<FInstancedStruct>& SaveData)
{
	if (SaveData.IsEmpty()) return;
	if (!SaveData[0].IsValid() || SaveData[0].GetScriptStruct() != FVariantDataStruct::StaticStruct())
	{
		UE_LOG(LogTemp, Warning, TEXT("Unexpected struct type in SaveData[0]."));
		return;
	}
	const FVariantDataStruct& Data = SaveData[0].Get<FVariantDataStruct>();
	if (Data.BoolValue) PloughSoil();
}

void UPlotPloughComponent::SaveComponentState_Implementation(TArray<FInstancedStruct>& OutSaveData)
{
	FInstancedStruct DataStruct;
	DataStruct.InitializeAs<FVariantDataStruct>();
	FVariantDataStruct* VariantData = DataStruct.GetMutablePtr<FVariantDataStruct>();
	VariantData->BoolValue = bHasBeenPloughed;
	OutSaveData.Add(DataStruct);
}


