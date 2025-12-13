// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveableActor.h"
#include "DataTypes/SlotStruct.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "Interfaces/SaveableInterface.h"
#include "FarmingPlot.generated.h"

class UPlantWateringComponent;
class UPlantBugComponent;
class UPlantWeedComponent;
class UPlantHealthComponent;
class UPlotPloughComponent;
class UBoxComponent;
class UPlantGrowthComponent;

UENUM(BlueprintType)
enum class EPlotInteractions : uint8
{
	plantSeed,
	water,
	harvest,
	plough,
	weed,
	useHerbicide,
	usePesticide,
	useFertilizer,
	clearDeadPlant,	
	none
};

UCLASS(Blueprintable)
class ROOFTOPGARDEN_API AFarmingPlot : public ASaveableActor, public IInteractable
{
	GENERATED_BODY()

public:
	AFarmingPlot();

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Default")
	TObjectPtr<UBoxComponent> Box;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Default")
	TObjectPtr<UBoxComponent> CropBox;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Default")
	TObjectPtr<UAudioComponent> HarvestSound;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Default")
	TObjectPtr<UPlantGrowthComponent> PlantGrowthComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Default")
	TObjectPtr<UPlantWateringComponent> PlantWateringComponent;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Default")
	TObjectPtr<UPlotPloughComponent> PlotPloughComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Default")
	TObjectPtr<UPlantHealthComponent> PlantHealthComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Default")
	TObjectPtr<UPlantWeedComponent> PlantWeedComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Default")
	TObjectPtr<UPlantBugComponent> PlantBugComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Default")
	TObjectPtr<UStaticMeshComponent> PlantMesh;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Default")
	TObjectPtr<UStaticMeshComponent> PlotMesh;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Default")
	bool bHasBeenWatered = false;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Default")
	bool bHasPlant = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Default")
	bool bHasDeadPlant = false;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Default")
	TObjectPtr<UStaticMesh> DeadPlantMesh;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Default")
	EPlotInteractions GetCurrentInteraction(FSlotStruct HeldItem);
	
	virtual void Interact_Implementation(AActor* InteractingActor, FSlotStruct HeldItem) override;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionFailed, FText, FailText);
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly)
	FOnInteractionFailed OnInteractionFailed;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
private:
	UFUNCTION()
	void OnDayPassed();

	UFUNCTION()
	FText GetErrorMessage(FSlotStruct HeldItem) const;
	
	UFUNCTION()
	void UpdatePlantMesh(UStaticMesh* NewMesh);
	
	UFUNCTION()
	void ResetPlot();

	UFUNCTION()
	void OnPlantDeath();
	
	UFUNCTION()
	void SetComponentsActive(bool bIsActive) const;

	virtual void LoadState_Implementation(const TArray<FInstancedStruct>& SaveData) override;
	virtual void SaveState_Implementation(TArray<FInstancedStruct>& OutSaveData) override;

	FName CurrentProduceID = NAME_Name;
};
