// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlantWateringComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROOFTOPGARDEN_API UPlantWateringComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	void BeginPlay();
	
public:	
	// Sets default values for this component's properties
	UPlantWateringComponent();

	bool bHasBeenWatered = false;

	UFUNCTION(BlueprintCallable)
	void Water();

	UFUNCTION()
	void OnDayPassed();
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWatered);
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly)
	FOnWatered OnWatered;
};
