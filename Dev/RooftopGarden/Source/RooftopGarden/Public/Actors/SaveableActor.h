// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/SaveableInterface.h"
#include "SaveableActor.generated.h"

UCLASS()
class ROOFTOPGARDEN_API ASaveableActor : public AActor, public ISaveableInterface
{
	GENERATED_BODY()
	
public:	
	ASaveableActor();

	virtual FGuid GetSaveID_Implementation() const override;
	virtual void SetSaveID_Implementation(const FGuid& NewID) override;
	virtual void PostEditImport() override;	
	
protected:
	UPROPERTY(BlueprintReadOnly)
	FGuid SaveID;

};
