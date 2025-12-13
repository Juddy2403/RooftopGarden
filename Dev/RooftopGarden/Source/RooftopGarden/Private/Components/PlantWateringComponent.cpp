#include "Components/PlantWateringComponent.h"
#include "Components/PlantHealthComponent.h"
#include "Game/TimeSubsystem.h"

void UPlantWateringComponent::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetSubsystem<UTimeSubsystem>()->OnDayEnded.AddDynamic(this, &UPlantWateringComponent::OnDayPassed);
}

UPlantWateringComponent::UPlantWateringComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlantWateringComponent::Water()
{
	if (bHasBeenWatered) return;
	bHasBeenWatered = true;
	OnWatered.Broadcast();
}

void UPlantWateringComponent::OnDayPassed()
{
	if (!IsActive()) return;
	if (!bHasBeenWatered) GetOwner()->GetComponentByClass<UPlantHealthComponent>()->TakeDamage();
	bHasBeenWatered = false;
}

