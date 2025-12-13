// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveableComponent.h"
#include "Components/ActorComponent.h"
#include "PlotPloughComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROOFTOPGARDEN_API UPlotPloughComponent : public USaveableComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlotPloughComponent();
	
	UFUNCTION(BlueprintCallable)
	void PloughSoil();

	UFUNCTION(BlueprintCallable)
	void ResetSoil();

	UFUNCTION(BlueprintCallable)
	bool HasBeenPloughed() const;
	
	virtual void LoadComponentState_Implementation(const TArray<FInstancedStruct>& SaveData) override;
	virtual void SaveComponentState_Implementation(TArray<FInstancedStruct>& OutSaveData) override;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPloughed);
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly)
	FOnPloughed OnPloughed;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPloughReset);
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly)
	FOnPloughReset OnPloughReset;
private:
	bool bHasBeenPloughed = false;
};
