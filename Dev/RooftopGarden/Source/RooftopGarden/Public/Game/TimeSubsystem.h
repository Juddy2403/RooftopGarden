// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimeSubsystem.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class ROOFTOPGARDEN_API UTimeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) // set in the Project Game settings
	float TimeSpeed;
	
	UPROPERTY(BlueprintReadOnly)
	float CurrentTime;

	UPROPERTY(BlueprintReadOnly)
	int CurrentDay = 1;

	UFUNCTION(BlueprintCallable)
	float GetSunRotation() const;

	UFUNCTION(BlueprintCallable)
	void ResetDay();

	UFUNCTION(BlueprintCallable)
	FString GetTimeString() const;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDayEnded);
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly)
	FOnDayEnded OnDayEnded;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMidnightReached);
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly)
	FOnMidnightReached OnMidnightReached;
private:
	FTimerHandle TimerHandle;
	
	UPROPERTY(EditDefaultsOnly) // set in the Project Game settings
	float TickInterval;

	UPROPERTY(EditDefaultsOnly) // set in the Project Game settings
	float DayStart;
	
	UFUNCTION()
	void AdvanceTime();

	void SetupTimer();
};
