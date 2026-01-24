#include "Actors/FarmingPlot.h"

#include "InstancedStruct.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/PlantBugComponent.h"
#include "Components/PlantGrowthComponent.h"
#include "Components/PlantHealthComponent.h"
#include "Components/PlantWateringComponent.h"
#include "Components/PlantWeedComponent.h"
#include "Components/PlotPloughComponent.h"
#include "DataTypes/ItemStruct.h"
#include "DataTypes/SaveDataStructs/FarmingPlotDataStruct.h"
#include "Game/TimeSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/UtilitiesLibrary.h"

class UTimeSubsystem;

AFarmingPlot::AFarmingPlot()
{
	PrimaryActorTick.bCanEverTick = false;
	// Creating all objects
	PlotMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlotMesh"));
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	CropBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CropBox"));
	HarvestSound = CreateDefaultSubobject<UAudioComponent>(TEXT("HarvestSound"));
	PlantMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	PlantWateringComponent = CreateDefaultSubobject<UPlantWateringComponent>(TEXT("PlantWateringComponent"));
	PlotPloughComponent = CreateDefaultSubobject<UPlotPloughComponent>(TEXT("PlotPloughComponent"));
	PlantHealthComponent = CreateDefaultSubobject<UPlantHealthComponent>(TEXT("PlantHealthComponent"));
	PlantWeedComponent = CreateDefaultSubobject<UPlantWeedComponent>(TEXT("PlantWeedComponent"));
	PlantBugComponent = CreateDefaultSubobject<UPlantBugComponent>(TEXT("PlantBugComponent"));
	PlantGrowthComponent = CreateDefaultSubobject<UPlantGrowthComponent>(TEXT("PlantGrowthComponent"));

	// Root needs to be the plot mesh cause of the pivot
	RootComponent = PlotMesh;
	PlantMesh->AttachToComponent(PlotMesh, FAttachmentTransformRules::KeepRelativeTransform);
	Box->AttachToComponent(PlotMesh, FAttachmentTransformRules::KeepRelativeTransform);
	CropBox->AttachToComponent(PlotMesh, FAttachmentTransformRules::KeepRelativeTransform);
	CropBox->SetBoxExtent(FVector(0.f, 0.f, 0.f), false);

	SetComponentsActive(false);
}

EPlotInteractions AFarmingPlot::GetCurrentInteraction_Implementation(FSlotStruct HeldItem) 
{
	FName ProduceID{};
	if (bHasPlant && PlantGrowthComponent->IsCollectable(ProduceID)) return EPlotInteractions::harvest;
	// Get currently held item
	if (HeldItem.ItemID.IsNone()) return EPlotInteractions::none;

	if (!bHasPlant && !bHasDeadPlant && HeldItem.ItemType == EItemType::Seed && PlotPloughComponent->HasBeenPloughed())
		return EPlotInteractions::plantSeed;

	if (HeldItem.ItemType == EItemType::Tool)
	{
		if (HeldItem.ItemID.Compare("tiller") == 0 && PlantWeedComponent->HasWeeds()) return EPlotInteractions::weed;
		if (HeldItem.ItemID.Compare("tiller") == 0 && bHasDeadPlant) return EPlotInteractions::clearDeadPlant;
		if (HeldItem.ItemID.Compare("watering_can") == 0 && !PlantWateringComponent->bHasBeenWatered) return EPlotInteractions::water;
		if (HeldItem.ItemID.Compare("shovel") == 0 && !bHasPlant && !PlotPloughComponent->HasBeenPloughed())
			return
				EPlotInteractions::plough;
		if (HeldItem.ItemID.Compare("pesticide") == 0 && !PlantBugComponent->bHasPesticide)
			return
				EPlotInteractions::usePesticide;
		if (HeldItem.ItemID.Compare("herbicide") == 0 && !PlantWeedComponent->bHasHerbicide)
			return
				EPlotInteractions::useHerbicide;
		if (HeldItem.ItemID.Compare("fertilizer") == 0 && !PlantGrowthComponent->bHasFertilizer)
			return
				EPlotInteractions::useFertilizer;
	}
	return EPlotInteractions::none;
}

void AFarmingPlot::Interact_Implementation(AActor* InteractingActor, FSlotStruct HeldItem)
{
	// return if no valid interaction available
	const EPlotInteractions CurrentInteractionEnum = GetCurrentInteraction(HeldItem);
	
	if (CurrentInteractionEnum == EPlotInteractions::none)
	{
		FText FailureReason = GetErrorMessage(HeldItem);
		if (!FailureReason.IsEmpty()) OnInteractionFailed.Broadcast(FailureReason);
		return;
	}
	if (CurrentInteractionEnum == EPlotInteractions::water) return;

	// Get active slot info to check for seed
	const auto InventoryComp = InteractingActor->GetComponentByClass<UInventoryComponent>();

	switch (CurrentInteractionEnum)
	{
	case EPlotInteractions::plantSeed:
		{
			if (!IsValid(InventoryComp)) return;
			// Fetch the plant info struct for held item
			FPlantInfoStruct PlantInfoStruct{};
			bool Success = UUtilitiesLibrary::GetActivePlantInfoStruct(InventoryComp, PlantInfoStruct);
			if (!Success) return;

			// Plant the seed
			SetComponentsActive(true);
			InventoryComp->RemoveFromActiveSlot(1);
			PlantGrowthComponent->UpdatePlantInfo(PlantInfoStruct);
			bHasPlant = true;
			CurrentProduceID = PlantInfoStruct.ProduceID;
		}
		break;
	case EPlotInteractions::harvest:
		{
			if (!IsValid(InventoryComp)) return;
			const bool bSuccess = InventoryComp->AddToInventory(CurrentProduceID, 1);
			if (!bSuccess) return;
			HarvestSound->Play();
			ResetPlot();
		}
		break;
	case EPlotInteractions::clearDeadPlant:
		{
			// Clear plant only when the interacting actor is the tiller tool, thus the inventory is null
			if (IsValid(InventoryComp)) return;
			bHasDeadPlant = false;
			PlantMesh->SetStaticMesh(nullptr);
		}
		break;
	default: break;
	}
}

void AFarmingPlot::BeginPlay()
{
	Super::BeginPlay();
	PlantHealthComponent->OnDeath.AddUniqueDynamic(this, &AFarmingPlot::OnPlantDeath);
	PlantGrowthComponent->OnMeshUpdated.AddUniqueDynamic(this, &AFarmingPlot::UpdatePlantMesh);
}

void AFarmingPlot::UpdatePlantMesh(UStaticMesh* NewMesh)
{
	PlantMesh->SetStaticMesh(NewMesh);
	if (PlantGrowthComponent->CurrentGrowthStage == 0) 
		PlantMesh->SetMaterial(0, NewMesh->GetMaterial(0));
	FName ProduceID{};
	if (!PlantGrowthComponent->IsCollectable(ProduceID)) return;
	if (!IsValid(NewMesh)) return;
	const FBoxSphereBounds LocalBounds = NewMesh->GetBoundingBox();
	CropBox->SetBoxExtent(LocalBounds.BoxExtent, false);
	// set the crop box location to be at the center of the mesh
	CropBox->SetRelativeLocation(PlantMesh->GetRelativeLocation() + FVector(0.f, 0.f, LocalBounds.BoxExtent.Z));
}

FText AFarmingPlot::GetErrorMessage(FSlotStruct HeldItem) const
{
	if (!bHasPlant && !bHasDeadPlant && HeldItem.ItemType == EItemType::Seed && !PlotPloughComponent->HasBeenPloughed())
		return FText::FromString("Soil is not ploughed");

	if (HeldItem.ItemType == EItemType::Tool)
	{
		if (HeldItem.ItemID.Compare("tiller") == 0 && !PlantWeedComponent->HasWeeds() && !bHasDeadPlant) 
			return FText::FromString("No weeds");
		if (HeldItem.ItemID.Compare("watering_can") == 0 && PlantWateringComponent->bHasBeenWatered)
			return FText::FromString("Soil is already watered");
		if (HeldItem.ItemID.Compare("shovel") == 0 && !bHasPlant && PlotPloughComponent->HasBeenPloughed())
			return FText::FromString("Soil is already ploughed");
	}
	return FText::GetEmpty();
}

void AFarmingPlot::ResetPlot()
{
	bHasPlant = false;
	PlantMesh->SetStaticMesh(nullptr);
	CropBox->SetBoxExtent(FVector(0.f, 0.f, 0.f));
	PlotPloughComponent->ResetSoil();
	CurrentProduceID = NAME_None;
	PlantBugComponent->Reset();
	PlantWeedComponent->Reset();
	PlantHealthComponent->Reset();
	SetComponentsActive(false);
}

void AFarmingPlot::OnPlantDeath()
{
	ResetPlot();
	bHasDeadPlant = true;
	if (IsValid(DeadPlantMesh))
	{
		PlantMesh->SetStaticMesh(DeadPlantMesh);
		PlantMesh->SetMaterial(0, DeadPlantMesh->GetMaterial(0));
	}
}

void AFarmingPlot::SetComponentsActive(bool bIsActive) const
{
	PlantGrowthComponent->SetActive(bIsActive);
	PlotPloughComponent->SetActive(bIsActive);
	PlantHealthComponent->SetActive(bIsActive);
	PlantWeedComponent->SetActive(bIsActive);
	PlantBugComponent->SetActive(bIsActive);
	PlantWateringComponent->SetActive(bIsActive);
}

void AFarmingPlot::LoadState_Implementation(const TArray<FInstancedStruct>& SaveData)
{
	if (SaveData.IsEmpty()) return;
	FFarmingPlotDataStruct DataStruct = SaveData[0].Get<FFarmingPlotDataStruct>();
	bHasDeadPlant = DataStruct.HasDeadPlant;
	if (bHasDeadPlant && IsValid(DeadPlantMesh)) PlantMesh->SetStaticMesh(DeadPlantMesh);
	bHasPlant = DataStruct.HasPlant;
	CurrentProduceID = DataStruct.CurrentProduceID;
	if (bHasPlant) SetComponentsActive(true);
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		PlantBugComponent->OnDayPassed();
		PlantWeedComponent->OnDayPassed();
	});
}

void AFarmingPlot::SaveState_Implementation(TArray<FInstancedStruct>& OutSaveData)
{
	FInstancedStruct DataStruct;
	DataStruct.InitializeAs<FFarmingPlotDataStruct>();
	FFarmingPlotDataStruct* PlotData = DataStruct.GetMutablePtr<FFarmingPlotDataStruct>();
	PlotData->HasDeadPlant = bHasDeadPlant;
	PlotData->HasPlant = bHasPlant;
	PlotData->CurrentProduceID = CurrentProduceID;
	OutSaveData.Add(DataStruct);
}
