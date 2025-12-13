#pragma once

#include "CoreMinimal.h"
#include "VariantDataStruct.generated.h"

USTRUCT(BlueprintType)
struct FVariantDataStruct
{
	GENERATED_BODY()
	FVariantDataStruct() = default;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	bool BoolValue = false;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	int32 IntValue = 0;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	float FloatValue = 0.f;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FName NameValue = NAME_None;
	
	UPROPERTY(SaveGame, BlueprintReadWrite)
	FString StringValue = "";
	
	UPROPERTY(SaveGame, BlueprintReadWrite)
	FGuid ID = FGuid();
};