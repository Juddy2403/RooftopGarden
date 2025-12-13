#include "Components/PhysicsItemComponent.h"

#include "Components/BoxComponent.h"
#include "Engine/StaticMeshActor.h"

UPhysicsItemComponent::UPhysicsItemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UPhysicsItemComponent::HasReachedLimit() const
{
	if (SpawnedActors.IsEmpty()) return false;
	if (!IsValid(SpawnLimitBox))
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnLimitBox is not valid in PhysicsItemComponent attached to %s"), *GetOwner()->GetName());
		return false;
	}
	TArray<AActor*> OverlappingActors;
	SpawnLimitBox->GetOverlappingActors(OverlappingActors, AStaticMeshActor::StaticClass());
	return !OverlappingActors.IsEmpty();
}

bool UPhysicsItemComponent::DiscardLastItem()
{
	if (SpawnedActors.IsEmpty()) return false;
	if (TotalHeldItems > SpawnedActors.Num())
	{
		--TotalHeldItems;
		return true;
	}
	AStaticMeshActor* ActorToDestroy = SpawnedActors.Pop();
	if (!IsValid(ActorToDestroy)) return false;
	ActorToDestroy->Destroy();
	--TotalHeldItems;
	return true;
}

void UPhysicsItemComponent::Reset()
{
	for (auto SpawnedActor : SpawnedActors)
	{
		if (IsValid(SpawnedActor)) SpawnedActor->Destroy();
	}
	SpawnedActors.Empty();
	TotalHeldItems = 0;
}

FVector UPhysicsItemComponent::GetRandomSpawnPoint() const
{
	if (!IsValid(SpawnAreaBox))
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnAreaBox is not valid in PhysicsItemComponent attached to %s"), *GetOwner()->GetName());
		return FVector::ZeroVector;
	}
	const FBox Box = FBox::BuildAABB(SpawnAreaBox->GetComponentLocation(), SpawnAreaBox->GetScaledBoxExtent());
	return FMath::RandPointInBox(Box);
}

bool UPhysicsItemComponent::SpawnAndSetupActor(UStaticMesh* ActorMesh)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
	auto* SpawnedActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
	                                                              GetRandomSpawnPoint(),
	                                                              FRotator::ZeroRotator, SpawnParameters);
	if (!IsValid(SpawnedActor)) return false;
	SpawnedActor->SetMobility(EComponentMobility::Movable);
	SpawnedActor->GetStaticMeshComponent()->SetStaticMesh(ActorMesh);
	SpawnedActor->GetStaticMeshComponent()->SetSimulatePhysics(true);
	SpawnedActor->GetStaticMeshComponent()->SetGenerateOverlapEvents(true);
	SpawnedActors.Add(SpawnedActor);
	++TotalHeldItems;
	return true;
}

bool UPhysicsItemComponent::SpawnActorIfWithinLimit(UStaticMesh* ActorMesh)
{
	if (HasReachedLimit())
	{
		++TotalHeldItems;
		return false;
	}
	return SpawnAndSetupActor(ActorMesh);
}
