// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveableComponent.h"
#include "Components/ActorComponent.h"
#include "PlantWeedComponent.generated.h"

class AStaticMeshActor;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROOFTOPGARDEN_API UPlantWeedComponent : public USaveableComponent
{
	GENERATED_BODY()

public:	
	UPlantWeedComponent();

	UFUNCTION(BlueprintCallable)
	bool HasWeeds() const { return SpawnedWeeds.Num() > 0; }

	UFUNCTION(BlueprintCallable)
	void ClearOneWeed();

	UFUNCTION(BlueprintCallable)
	bool UseHerbicide();

	UFUNCTION(BlueprintCallable)
	void Reset();
	
	UPROPERTY(EditDefaultsOnly)
	int32 SpawnChanceDenominator = 2;

	UPROPERTY(BlueprintReadOnly)
	bool bHasHerbicide = false;
	
	UFUNCTION()
	void OnDayPassed();
protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditDefaultsOnly)
	FVector WeedSpawnOffset = FVector(0.f, 0.f, 40.f);

	UPROPERTY(EditDefaultsOnly)
	float SpawnSideRange = 30;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMesh> WeedMesh;

	UPROPERTY()
	TArray<AStaticMeshActor*> SpawnedWeeds;

	UFUNCTION()
	void SpawnWeed();

public:
	virtual void LoadComponentState_Implementation(const TArray<FInstancedStruct>& SaveData) override;
	virtual void SaveComponentState_Implementation(TArray<FInstancedStruct>& OutSaveData) override;
};
