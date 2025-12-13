// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveableComponent.h"
#include "Components/ActorComponent.h"
#include "DataTypes/PlantInfoStruct.h"
#include "PlantGrowthComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ROOFTOPGARDEN_API UPlantGrowthComponent : public USaveableComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlantGrowthComponent();

	UFUNCTION(BlueprintCallable)
	void UpdatePlantInfo(FPlantInfoStruct NewPlantInfo);

	UFUNCTION(BlueprintCallable)
	UStaticMesh* GetCurrentMesh();

	UFUNCTION(BlueprintCallable)
	bool UseFertilizer();

	UFUNCTION(BlueprintCallable)
	bool IsCollectable(FName& ProduceID) const;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeshUpdated, UStaticMesh*, NewMesh);
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly)
	FOnMeshUpdated OnMeshUpdated;

	UPROPERTY(BlueprintReadOnly)
	bool bHasFertilizer = false;
	
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentGrowthStage = 0;
	
	virtual void LoadComponentState_Implementation(const TArray<FInstancedStruct>& SaveData) override;
	virtual void SaveComponentState_Implementation(TArray<FInstancedStruct>& OutSaveData) override;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void TryGrowPlant();
	
	UFUNCTION()
	void OnDayPassed();
	
	FPlantInfoStruct PlantInfo;
};
