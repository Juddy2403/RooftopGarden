#pragma once

#include "CoreMinimal.h"
#include "ComponentSaveStruct.h"
#include "InstancedStruct.h"
#include "SaveStruct.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FSaveStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FTransform Transform;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TArray<FInstancedStruct> SaveData;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TMap<FGuid, FComponentSaveStruct> ComponentData;
};
