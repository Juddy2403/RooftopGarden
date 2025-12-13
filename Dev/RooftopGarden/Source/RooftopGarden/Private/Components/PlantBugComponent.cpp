// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PlantBugComponent.h"

#include "InstancedStruct.h"
#include "Actors/FarmingPlot.h"
#include "Components/PlantGrowthComponent.h"
#include "Components/PlantHealthComponent.h"
#include "DataTypes/SaveDataStructs/VariantDataStruct.h"
#include "Game/TimeSubsystem.h"

class UTimeSubsystem;

UPlantBugComponent::UPlantBugComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UPlantBugComponent::UsePesticide()
{
	if (bHasPesticide) return false;
	bHasPesticide = true;
	if (!SpawnedBugs.IsEmpty())
	{
		// Clear existing bugs
		for (const auto BugActor : SpawnedBugs)
		{
			if (!IsValid(BugActor)) continue;
			// If the bug actor has an owner, it means it was picked by the player
			if (IsValid(BugActor->GetOwner())) continue;
			BugActor->Destroy();
		}
		SpawnedBugs.Empty();
	}
	return true;
}

void UPlantBugComponent::Reset()
{
	bHasPesticide = false;
}

void UPlantBugComponent::BugPickedUp(AActor* BugActor)
{
	SpawnedBugs.Remove(BugActor);
	if (SpawnedBugs.IsEmpty()) OnBugsCleared.Broadcast();
}


void UPlantBugComponent::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetSubsystem<UTimeSubsystem>()->OnDayEnded.AddDynamic(this, &UPlantBugComponent::OnDayPassed);
}

void UPlantBugComponent::OnDayPassed()
{
	// If bugs were spawned and not cleared, damage the plant
	if (!SpawnedBugs.IsEmpty())
	{
		bool bHadBugsUncleared = false;
		// Clear existing bugs
		for (const auto BugActor : SpawnedBugs)
		{
			if (!IsValid(BugActor)) continue;
			// If the bug actor has an owner, it means it was picked by the player
			if (IsValid(BugActor->GetOwner())) continue;
			BugActor->Destroy();
			bHadBugsUncleared = true;
		}
		if (bHadBugsUncleared && IsActive()) GetOwner()->GetComponentByClass<UPlantHealthComponent>()->TakeDamage();
		SpawnedBugs.Empty();
		OnBugsCleared.Broadcast();
	}
	if (!IsActive()) return;
	if (bHasPesticide) return;
	const int32 SpawnChance = FMath::RandRange(1, SpawnChanceDenominator);

	// Spawn bugs next frame to make sure they get spawned on the current growth mesh
	if (SpawnChance == 1)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
			{
				FName ProduceID;
				// Do not spawn bugs if plant is harvestable
				if (GetOwner()->GetComponentByClass<UPlantGrowthComponent>()->IsCollectable(ProduceID)) return;
				SpawnBugs();
			});
		});
	}
}

void UPlantBugComponent::SpawnBugs()
{
	if (BugObjects.IsEmpty()) return;
	UWorld* World = GetWorld();
	const AFarmingPlot* PlotOwner = Cast<AFarmingPlot>(GetOwner());
	if (!PlotOwner || !PlotOwner->PlantMesh) return;

	const auto PlantSocketNames = PlotOwner->PlantMesh->GetAllSocketNames();
	if (PlantSocketNames.IsEmpty()) return;
	bool bSpawnedBugs = false;
	for (const auto SocketName : PlantSocketNames)
	{
		// The handle is NOT a bug socket. Faster to compare like this instead of string contains bug 
		if (SocketName.IsNone() || SocketName.Compare("Handle") == 0) continue;
		// 1 in 3 chance not to use this socket
		const int32 BugSpawnChance = FMath::RandRange(0, 2);
		if (BugSpawnChance == 0) continue;
		bSpawnedBugs = true;
		const FTransform SocketTransform = PlotOwner->PlantMesh->GetSocketTransform(SocketName);
		const int32 BugIndex = FMath::RandRange(0, BugObjects.Num() - 1);

		AActor* Item = World->SpawnActor<AActor>(
			BugObjects[BugIndex],
			SocketTransform
		);
		if (!Item) return;
		Item->SetOwner(GetOwner());
		SpawnedBugs.Add(Item);
	}
	if (bSpawnedBugs) OnBugsSpawned.Broadcast();
}

void UPlantBugComponent::LoadComponentState_Implementation(const TArray<FInstancedStruct>& SaveData)
{
	if (SaveData.IsEmpty()) return;
	if (!SaveData[0].IsValid() || SaveData[0].GetScriptStruct() != FVariantDataStruct::StaticStruct())
	{
		UE_LOG(LogTemp, Warning, TEXT("Unexpected struct type in SaveData[0]."));
		return;
	}
	const FVariantDataStruct& Data = SaveData[0].Get<FVariantDataStruct>();
	bHasPesticide = Data.BoolValue;
}

void UPlantBugComponent::SaveComponentState_Implementation(TArray<FInstancedStruct>& OutSaveData)
{
	FInstancedStruct DataStruct;
	DataStruct.InitializeAs<FVariantDataStruct>();
	FVariantDataStruct* VariantData = DataStruct.GetMutablePtr<FVariantDataStruct>();
	VariantData->BoolValue = bHasPesticide;
	OutSaveData.Add(DataStruct);
}
