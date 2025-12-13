// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/PlantGrowthComponent.h"

#include "InstancedStruct.h"
#include "Actors/FarmingPlot.h"
#include "Components/PlantHealthComponent.h"
#include "DataTypes/SaveDataStructs/VariantDataStruct.h"
#include "Game/TimeSubsystem.h"

// Sets default values for this component's properties
UPlantGrowthComponent::UPlantGrowthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UPlantGrowthComponent::TryGrowPlant()
{
	if (CurrentGrowthStage >= PlantInfo.GrowthStageCount - 1) return;
	++CurrentGrowthStage;
	OnMeshUpdated.Broadcast(GetCurrentMesh());
}

void UPlantGrowthComponent::OnDayPassed()
{
	if (!IsActive()) return;

	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		if (GetOwner()->GetComponentByClass<UPlantHealthComponent>()->DidTakeDamageThisCycle()) return;
		if (bHasFertilizer)
		{
			TryGrowPlant();
			bHasFertilizer = false;
		}
		TryGrowPlant();
	});
}

void UPlantGrowthComponent::UpdatePlantInfo(FPlantInfoStruct NewPlantInfo)
{
	bHasFertilizer = false;
	PlantInfo = NewPlantInfo;
	CurrentGrowthStage = 0;
	OnMeshUpdated.Broadcast(GetCurrentMesh());
}

UStaticMesh* UPlantGrowthComponent::GetCurrentMesh()
{
	return PlantInfo.GrowthStageMeshes.IsValidIndex(CurrentGrowthStage)
		       ? PlantInfo.GrowthStageMeshes[CurrentGrowthStage]
		       : nullptr;
}

bool UPlantGrowthComponent::UseFertilizer()
{
	if (bHasFertilizer) return false;
	bHasFertilizer = true;
	return true;
}

bool UPlantGrowthComponent::IsCollectable(FName& ProduceID) const
{
	if (PlantInfo.GrowthStageCount - 1 != CurrentGrowthStage) return false;
	ProduceID = PlantInfo.ProduceID;
	return true;
}

void UPlantGrowthComponent::LoadComponentState_Implementation(const TArray<FInstancedStruct>& SaveData)
{
	AFarmingPlot* PlotOwner = Cast<AFarmingPlot>(GetOwner());
	if (!PlotOwner->bHasPlant) return;

	if (SaveData.Num() < 2) return;

	const FPlantInfoStruct LoadedPlantInfo = SaveData[0].Get<FPlantInfoStruct>();
	PlantInfo = LoadedPlantInfo;
	const FVariantDataStruct LoadedVariantData = SaveData[1].Get<FVariantDataStruct>();
	CurrentGrowthStage = LoadedVariantData.IntValue;
	OnMeshUpdated.Broadcast(GetCurrentMesh());
}

void UPlantGrowthComponent::SaveComponentState_Implementation(TArray<FInstancedStruct>& OutSaveData)
{
	FInstancedStruct DataStruct;
	DataStruct.InitializeAs<FPlantInfoStruct>();
	FPlantInfoStruct* PlantData = DataStruct.GetMutablePtr<FPlantInfoStruct>();
	PlantData->GrowthStageCount = PlantInfo.GrowthStageCount;
	PlantData->GrowthStageMeshes = PlantInfo.GrowthStageMeshes;
	PlantData->ProduceID = PlantInfo.ProduceID;
	OutSaveData.Add(DataStruct);
	DataStruct.InitializeAs<FVariantDataStruct>();
	FVariantDataStruct* VariantData = DataStruct.GetMutablePtr<FVariantDataStruct>();
	VariantData->IntValue = CurrentGrowthStage;
	OutSaveData.Add(DataStruct);
}

void UPlantGrowthComponent::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetSubsystem<UTimeSubsystem>()->OnDayEnded.AddDynamic(this, &UPlantGrowthComponent::OnDayPassed);
}
