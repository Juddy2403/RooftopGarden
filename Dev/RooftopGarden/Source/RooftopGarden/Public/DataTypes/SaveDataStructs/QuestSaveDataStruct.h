#pragma once

#include "CoreMinimal.h"
#include "QuestSaveDataStruct.generated.h"

USTRUCT(BlueprintType)
struct FQuestSaveDataStruct
{
	GENERATED_BODY()
	FQuestSaveDataStruct() = default;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	int32 QuestIDX = false;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	TMap <FName, int32> GivenItems;
};
