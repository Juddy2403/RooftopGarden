#include "Game/SaveGameObject.h"

#include "Game/TimeSubsystem.h"
#include "Interfaces/SaveableComponentInterface.h"
#include "Interfaces/SaveableInterface.h"
#include "Kismet/GameplayStatics.h"

void USaveGameObject::LoadGameData()
{
	UTimeSubsystem* TimeSub = GWorld->GetSubsystem<UTimeSubsystem>();
	TimeSub->CurrentDay = SavedDay;

	TArray<AActor*> ExistingSaveableActors;
	UGameplayStatics::GetAllActorsWithInterface(GWorld, USaveableInterface::StaticClass(), ExistingSaveableActors);

	TMap<FGuid, FSaveStruct> ActorsToSpawn = SavedActors;
	TMap<FGuid, AActor*> ActorMap;

	for (auto& Actor : ExistingSaveableActors)
	{
		FGuid SaveID = ISaveableInterface::Execute_GetSaveID(Actor);
		// Loading data for existing saved actors
		if (SavedActors.Contains(SaveID))
		{
			FSaveStruct& SaveData = SavedActors[SaveID];
			LoadExistingActorState(Actor, SaveData);
			ActorMap.Add(SaveID, Actor);
			ActorsToSpawn.Remove(SaveID);
		}
		// Destroying current actors that are not in the save data
		else Actor->Destroy();
	}

	// Spawning the saved actors that don't exist in the world
	for (auto& Actor : ActorsToSpawn)
	{
		FSaveStruct SaveData = Actor.Value;
		FGuid SaveID = Actor.Key;
		AActor* SpawnedActor = SpawnFromSaveData(SaveData);

		LoadSpawnedActor(SpawnedActor, SaveID, SaveData);
		ActorMap.Add(SaveID, SpawnedActor);
	}

	// Loading the player's component data
	AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(GWorld, 0);
	LoadComponentStates(PlayerActor, PlayerComponentData);

	// Restoring relationships for all saved actors
	GWorld->GetTimerManager().SetTimerForNextTick([ActorMap]()
	{
		for (auto& ActorPair : ActorMap)
		{
			ISaveableInterface::Execute_RestoreRelationships(ActorPair.Value, ActorMap);
		}

		AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(GWorld, 0);
		auto SaveableComponents = PlayerActor->GetComponentsByInterface(USaveableComponentInterface::StaticClass());
		for (auto& Component : SaveableComponents)
		{
			ISaveableComponentInterface::Execute_RestoreRelationships(Component, ActorMap);
		}
	});
}

void USaveGameObject::LoadSpawnedActor(AActor* SpawnedActor, FGuid SaveID, FSaveStruct SaveData)
{
	ISaveableInterface::Execute_LoadState(SpawnedActor, SaveData.SaveData);
	ISaveableInterface::Execute_SetSaveID(SpawnedActor, SaveID);

	if (SaveData.ComponentData.IsEmpty()) return;

	LoadComponentStates(SpawnedActor, SaveData.ComponentData);
}

void USaveGameObject::LoadComponentStates(AActor* SpawnedActor, TMap<FGuid, FComponentSaveStruct>& CompSaveData)
{
	auto SaveableComponents = SpawnedActor->GetComponentsByInterface(USaveableComponentInterface::StaticClass());
	for (auto& Component : CompSaveData)
	{
		FComponentSaveStruct ComponentSaveStruct = Component.Value;
		// Finding an existing component with the same class as the saved component
		UActorComponent* ExistingComp{};
		for (auto& Comp : SaveableComponents)
		{
			if (!Comp->IsA(ComponentSaveStruct.ComponentClass)) continue;

			ExistingComp = Comp;
			break;
		}
		if (!IsValid(ExistingComp)) continue;
		ISaveableComponentInterface::Execute_LoadComponentState(ExistingComp, ComponentSaveStruct.SaveData);
		ISaveableComponentInterface::Execute_SetComponentSaveID(ExistingComp, Component.Key);
		// Removing the component from the list so we can save the rest of the components correctly
		SaveableComponents.Remove(ExistingComp);
	}
}

void USaveGameObject::LoadExistingActorState(AActor*& Actor, FSaveStruct& SaveData)
{
	ISaveableInterface::Execute_LoadState(Actor, SaveData.SaveData);
	Actor->SetActorTransform(SaveData.Transform);

	if (SaveData.ComponentData.IsEmpty()) return;

	auto SaveableComponents = Actor->GetComponentsByInterface(USaveableComponentInterface::StaticClass());
	for (auto& Component : SaveableComponents)
	{
		FGuid ComponentSaveID = ISaveableComponentInterface::Execute_GetComponentSaveID(Component);
		if (SaveData.ComponentData.Contains(ComponentSaveID))
		{
			const FComponentSaveStruct& ComponentSaveData = SaveData.ComponentData[ComponentSaveID];
			ISaveableComponentInterface::Execute_LoadComponentState(Component, ComponentSaveData.SaveData);
		}
	}
}

void USaveGameObject::SaveGameData()
{
	UTimeSubsystem* TimeSub = GWorld->GetSubsystem<UTimeSubsystem>();
	SavedDay = TimeSub->CurrentDay;

	TArray<AActor*> SaveableActors;
	UGameplayStatics::GetAllActorsWithInterface(GWorld, USaveableInterface::StaticClass(), SaveableActors);

	SavedActors.Empty();

	for (auto& Actor : SaveableActors)
	{
		// Saving the actor's data
		TArray<FInstancedStruct> ActorData;
		ISaveableInterface::Execute_SaveState(Actor, ActorData);

		FSaveStruct SaveData;
		SaveData.ActorClass = Actor->GetClass();
		SaveData.Transform = Actor->GetTransform();
		SaveData.SaveData = ActorData;

		FGuid SaveID = ISaveableInterface::Execute_GetSaveID(Actor);
		SaveActorComponentsData(Actor, SaveData.ComponentData);
		SavedActors.Add(SaveID, SaveData);
	}

	// Save the player's component data
	AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(GWorld, 0);
	SaveActorComponentsData(PlayerActor, PlayerComponentData);
}

void USaveGameObject::SaveActorComponentsData(AActor*& Actor, TMap<FGuid, FComponentSaveStruct>& CompSaveData)
{
	auto SaveableComponents = Actor->GetComponentsByInterface(USaveableComponentInterface::StaticClass());

	// Saving the actor's component data
	for (auto& Component : SaveableComponents)
	{
		TArray<FInstancedStruct> ComponentData;
		ISaveableComponentInterface::Execute_SaveComponentState(Component, ComponentData);
		FGuid ComponentSaveID = ISaveableComponentInterface::Execute_GetComponentSaveID(Component);

		FComponentSaveStruct ComponentSaveData;
		ComponentSaveData.ComponentClass = Component->GetClass();
		ComponentSaveData.SaveData = ComponentData;
		CompSaveData.Add(ComponentSaveID, ComponentSaveData);
	}
}

AActor* USaveGameObject::SpawnFromSaveData(const FSaveStruct& SaveStruct) const
{
	UClass* Class = SaveStruct.ActorClass;
	AActor* NewActor = GWorld->SpawnActor<AActor>(Class, SaveStruct.Transform);

	return NewActor;
}
