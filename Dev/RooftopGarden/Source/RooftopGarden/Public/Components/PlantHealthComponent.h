// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveableComponent.h"
#include "Components/ActorComponent.h"
#include "PlantHealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROOFTOPGARDEN_API UPlantHealthComponent : public USaveableComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlantHealthComponent();

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int32 MaxHealth = 2;

	UFUNCTION(BlueprintCallable)
	void TakeDamage();

	UFUNCTION(BlueprintCallable)
	void Reset();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly)
	FOnDeath OnDeath;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSick);
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly)
	FOnSick OnSick;

	bool DidTakeDamageThisCycle() const { return bTookDamageThisCycle; }
	
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentHealth;
protected:
	void BeginPlay() override;
private:
	UFUNCTION()
	void OnDayPassed();

public:
	virtual void LoadComponentState_Implementation(const TArray<FInstancedStruct>& SaveData) override;
	virtual void SaveComponentState_Implementation(TArray<FInstancedStruct>& OutSaveData) override;

private:
	bool bTookDamageThisCycle = false;
	
};
