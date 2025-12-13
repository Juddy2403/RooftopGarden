// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveableInterface.generated.h"

struct FInstancedStruct;
struct FSaveStruct;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USaveableInterface : public UInterface
{
	GENERATED_BODY()
};

class ROOFTOPGARDEN_API ISaveableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SaveState(TArray<FInstancedStruct>& OutSaveData);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void LoadState(const TArray<FInstancedStruct>& SaveData);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FGuid GetSaveID() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetSaveID(const FGuid& NewID);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void RestoreRelationships(const TMap<FGuid, AActor*>& ActorMap);
};
