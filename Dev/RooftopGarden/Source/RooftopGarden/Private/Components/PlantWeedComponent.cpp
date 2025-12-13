#include "Components/PlantWeedComponent.h"

#include "InstancedStruct.h"
#include "Components/PlantGrowthComponent.h"
#include "Components/PlantHealthComponent.h"
#include "DataTypes/SaveDataStructs/VariantDataStruct.h"
#include "Engine/StaticMeshActor.h"
#include "Game/TimeSubsystem.h"

class UPlantHealthComponent;
class UTimeSubsystem;
// Sets default values for this component's properties
UPlantWeedComponent::UPlantWeedComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlantWeedComponent::ClearOneWeed()
{
	SpawnedWeeds.Pop()->Destroy();
}

bool UPlantWeedComponent::UseHerbicide()
{
	if (bHasHerbicide) return false;
	bHasHerbicide = true;
	if (HasWeeds())
	{
		for (auto Spawned : SpawnedWeeds)
			if (IsValid(Spawned)) Spawned->Destroy();
		SpawnedWeeds.Empty();
	}
	return true;
}

void UPlantWeedComponent::Reset()
{
	bHasHerbicide = false;
}

void UPlantWeedComponent::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetSubsystem<UTimeSubsystem>()->OnDayEnded.AddDynamic(this, &UPlantWeedComponent::OnDayPassed);
}

void UPlantWeedComponent::OnDayPassed()
{
	if (!IsActive()) return;
	if (HasWeeds()) GetOwner()->GetComponentByClass<UPlantHealthComponent>()->TakeDamage();
	if (bHasHerbicide) return;
	const int32 SpawnChance = FMath::RandRange(1, SpawnChanceDenominator);
	if (SpawnChance == 1)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
			{
				FName ProduceID;
				// Only spawn weed if the plant is not yet collectable
				if (!GetOwner()->GetComponentByClass<UPlantGrowthComponent>()->IsCollectable(ProduceID)) SpawnWeed();
			});
		});
	}
}

void UPlantWeedComponent::SpawnWeed()
{
	// Randomly spawn a weed mesh around the plant within the specified range
	if (!WeedMesh) return;
	AActor* Owner = GetOwner();
	UWorld* World = Owner->GetWorld();

	const float RandX = FMath::RandRange(-SpawnSideRange, SpawnSideRange);
	const float RandY = FMath::RandRange(-SpawnSideRange, SpawnSideRange);

	// Final spawn location
	const FVector SpawnLocation = Owner->GetActorLocation()
		+ FVector(RandX, RandY, 0.f)
		+ WeedSpawnOffset;

	// Spawn rotation (upward)
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	// Spawn the actor
	AStaticMeshActor* SpawnedWeed = World->SpawnActor<AStaticMeshActor>(
		AStaticMeshActor::StaticClass(),
		SpawnLocation,
		SpawnRotation
	);

	// Apply the mesh to the new actor
	if (SpawnedWeed && SpawnedWeed->GetStaticMeshComponent())
	{
		SpawnedWeed->SetMobility(EComponentMobility::Movable);
		SpawnedWeed->GetStaticMeshComponent()->SetStaticMesh(WeedMesh);
		SpawnedWeed->SetActorEnableCollision(false);
	}

	SpawnedWeeds.Add(SpawnedWeed);
}

void UPlantWeedComponent::LoadComponentState_Implementation(const TArray<FInstancedStruct>& SaveData)
{
	if (SaveData.IsEmpty()) return;
	if (!SaveData[0].IsValid() || SaveData[0].GetScriptStruct() != FVariantDataStruct::StaticStruct())
	{
		UE_LOG(LogTemp, Warning, TEXT("Unexpected struct type in SaveData[0]."));
		return;
	}
	const FVariantDataStruct& Data = SaveData[0].Get<FVariantDataStruct>();
	bHasHerbicide = Data.BoolValue;
}

void UPlantWeedComponent::SaveComponentState_Implementation(TArray<FInstancedStruct>& OutSaveData)
{
	FInstancedStruct DataStruct;
	DataStruct.InitializeAs<FVariantDataStruct>();
	FVariantDataStruct* VariantData = DataStruct.GetMutablePtr<FVariantDataStruct>();
	VariantData->BoolValue = bHasHerbicide;
	OutSaveData.Add(DataStruct);
}
