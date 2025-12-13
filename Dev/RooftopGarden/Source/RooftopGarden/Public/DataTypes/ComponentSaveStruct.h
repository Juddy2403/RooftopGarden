#pragma once

#include "CoreMinimal.h"
#include "InstancedStruct.h"
#include "ComponentSaveStruct.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FComponentSaveStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TSubclassOf<UActorComponent> ComponentClass;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TArray<FInstancedStruct> SaveData;
};
