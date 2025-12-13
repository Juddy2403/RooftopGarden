// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PlantHealthComponent.h"

#include "InstancedStruct.h"
#include "DataTypes/SaveDataStructs/VariantDataStruct.h"
#include "Game/TimeSubsystem.h"

UPlantHealthComponent::UPlantHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentHealth = MaxHealth;
}

void UPlantHealthComponent::TakeDamage()
{
	if (CurrentHealth <= 0) return;
	UE_LOG(LogTemp, Warning, TEXT("%s Plant took damage!"), *GetOwner()->GetName());
	bTookDamageThisCycle = true;
	CurrentHealth--;
	if (CurrentHealth == 0) OnDeath.Broadcast();
	else OnSick.Broadcast();
}

void UPlantHealthComponent::Reset()
{
	CurrentHealth = MaxHealth;
}

void UPlantHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetSubsystem<UTimeSubsystem>()->OnDayEnded.AddDynamic(this, &UPlantHealthComponent::OnDayPassed);
}

void UPlantHealthComponent::OnDayPassed()
{
	if (!IsActive()) return;
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			bTookDamageThisCycle = false;
		});
	});
}

void UPlantHealthComponent::LoadComponentState_Implementation(const TArray<FInstancedStruct>& SaveData)
{
	if (SaveData.IsEmpty()) return;
	if (!SaveData[0].IsValid() || SaveData[0].GetScriptStruct() != FVariantDataStruct::StaticStruct())
	{
		UE_LOG(LogTemp, Warning, TEXT("Unexpected struct type in SaveData[0]."));
		return;
	}
	const FVariantDataStruct& DataStruct = SaveData[0].Get<FVariantDataStruct>();
	CurrentHealth = DataStruct.IntValue;
}

void UPlantHealthComponent::SaveComponentState_Implementation(TArray<FInstancedStruct>& OutSaveData)
{
	FInstancedStruct DataStruct;
	DataStruct.InitializeAs<FVariantDataStruct>();
	FVariantDataStruct* VariantData = DataStruct.GetMutablePtr<FVariantDataStruct>();
	VariantData->IntValue = CurrentHealth;
	OutSaveData.Add(DataStruct);
}
