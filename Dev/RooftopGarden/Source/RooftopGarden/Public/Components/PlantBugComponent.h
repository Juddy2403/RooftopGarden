// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveableComponent.h"
#include "Components/ActorComponent.h"
#include "PlantBugComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROOFTOPGARDEN_API UPlantBugComponent : public USaveableComponent
{
	GENERATED_BODY()

public:	
	UPlantBugComponent();

	UFUNCTION(BlueprintCallable)
	bool UsePesticide();

	UFUNCTION(BlueprintCallable)
	void Reset();

	UFUNCTION(BlueprintCallable)
	void BugPickedUp(AActor* BugActor);
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int32 SpawnChanceDenominator;

	UPROPERTY(BlueprintReadOnly)
	bool bHasPesticide = false;
	
	UFUNCTION()
	void OnDayPassed();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBugsSpawned);
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly)
	FOnBugsSpawned OnBugsSpawned;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBugsCleared);
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly)
	FOnBugsCleared OnBugsCleared;
protected:
	virtual void BeginPlay() override;

private:
	
	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<AActor>> BugObjects;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedBugs;

	UFUNCTION()
	void SpawnBugs();

public:
	virtual void LoadComponentState_Implementation(const TArray<FInstancedStruct>& SaveData) override;
	virtual void SaveComponentState_Implementation(TArray<FInstancedStruct>& OutSaveData) override;
};
