// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveableComponentInterface.generated.h"

struct FInstancedStruct;
struct FComponentSaveStruct;
UINTERFACE(MinimalAPI)
class USaveableComponentInterface : public UInterface
{
	GENERATED_BODY()
};

class ROOFTOPGARDEN_API ISaveableComponentInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SaveComponentState(TArray<FInstancedStruct>& OutSaveData);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void LoadComponentState(const TArray<FInstancedStruct>& SaveData);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FGuid GetComponentSaveID() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetComponentSaveID(const FGuid& NewID);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void RestoreRelationships(const TMap<FGuid, AActor*>& ActorMap);
};
