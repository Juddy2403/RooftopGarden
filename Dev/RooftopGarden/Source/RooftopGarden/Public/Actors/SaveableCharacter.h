#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SaveableCharacter.generated.h"

UCLASS()
class ROOFTOPGARDEN_API ASaveableCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASaveableCharacter();
	
	virtual void PostInitializeComponents() override;

};
