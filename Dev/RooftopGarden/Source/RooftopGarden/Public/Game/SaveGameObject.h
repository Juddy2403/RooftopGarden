// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataTypes/SaveStruct.h"
#include "GameFramework/SaveGame.h"
#include "SaveGameObject.generated.h"

UCLASS(Blueprintable, BlueprintType)
class ROOFTOPGARDEN_API USaveGameObject : public USaveGame
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void LoadGameData();

	UFUNCTION(BlueprintCallable) 
	void SaveGameData();

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 SavedDay;
private:
	UPROPERTY(SaveGame)
	TMap<FGuid, FSaveStruct> SavedActors;

	UPROPERTY(SaveGame)
	TMap<FGuid, FComponentSaveStruct> PlayerComponentData;
	
	UFUNCTION()
	AActor* SpawnFromSaveData(const FSaveStruct& SaveStruct) const;

	UFUNCTION()
	void LoadExistingActorState(AActor*& Actor, FSaveStruct& SaveData);
	
	UFUNCTION()
	void SaveActorComponentsData(AActor*& Actor, TMap<FGuid, FComponentSaveStruct>& CompSaveData);

	UFUNCTION()
	void LoadSpawnedActor(AActor* SpawnedActor, FGuid SaveID, FSaveStruct SaveData);
	
	UFUNCTION()
	void LoadComponentStates(AActor* SpawnedActor, TMap<FGuid, FComponentSaveStruct>& CompSaveData);
};
