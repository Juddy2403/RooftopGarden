// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/SaveableComponentInterface.h"
#include "SaveableComponent.generated.h"


UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROOFTOPGARDEN_API USaveableComponent : public UActorComponent, public ISaveableComponentInterface
{
	GENERATED_BODY()

public:	
	USaveableComponent() = default;

	virtual FGuid GetComponentSaveID_Implementation() const override;
	virtual void SetComponentSaveID_Implementation(const FGuid& NewID) override;
protected:
	
	UPROPERTY(BlueprintReadOnly)
	FGuid SaveID;

};
