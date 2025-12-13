// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/TimeSubsystem.h"

#include "DataTypes/GameTimeSettings.h"

void UTimeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	const UGameTimeSettings* TimeSettings = GetDefault<UGameTimeSettings>();
	TimeSpeed = TimeSettings->TimeSpeed;
	DayStart = TimeSettings->DayStart;
	TickInterval = TimeSettings->TickInterval;
	
	//if somehow the timer persisted, reset it
	if (GetWorld()->GetTimerManager().IsTimerActive(TimerHandle)) GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	
	SetupTimer();
	CurrentTime = DayStart;
}

float UTimeSubsystem::GetSunRotation() const
{
	return CurrentTime / 24.f * 360.f + 90.f;
}

void UTimeSubsystem::SetupTimer()
{
	if (!GetWorld()->GetTimerManager().IsTimerActive(TimerHandle))
	{
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			this,
			&UTimeSubsystem::AdvanceTime,
			TickInterval,
			true
		);
	}
}

void UTimeSubsystem::ResetDay()
{
	CurrentTime = DayStart;
	++CurrentDay;
	OnDayEnded.Broadcast();
	SetupTimer();
}

FString UTimeSubsystem::GetTimeString() const
{
	// time is between 6 and 24. Output time in 12-hour format with AM/PM, rounding to 10mins: 6:00 AM, 6:10 AM, 6:20 AM, ..., 11:50 AM, 12:00 PM, 12:10 PM, ..., 5:50 PM
	int32 Hours = FMath::FloorToInt(CurrentTime);
	float MinuteFraction = (CurrentTime - Hours) * 60.f;

	int32 Minutes = FMath::RoundToInt(MinuteFraction / 10.f) * 10;

	// Handle overflow: 60 minutes should roll over to next hour
	if (Minutes == 60)
	{
		Minutes = 0;
		Hours += 1;
	}

	// Wrap around 24-hour clock if needed
	Hours = Hours % 24;

	FString AMPM = Hours >= 12 ? "PM" : "AM";
	int32 DisplayHour = Hours % 12;
	if (DisplayHour == 0 && AMPM == "PM") DisplayHour = 12;

	return FString::Printf(TEXT("%02d:%02d %s"), DisplayHour, Minutes, *AMPM);
}

void UTimeSubsystem::AdvanceTime()
{
	CurrentTime += TickInterval * TimeSpeed;
	// If the day is over, pause time passing.
	if (CurrentTime >= 24)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		OnMidnightReached.Broadcast();
	}
}
